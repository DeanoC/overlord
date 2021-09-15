package overlord

import ikuy_utils._
import overlord.Chip.{BitsDesc, RegisterBank, RegisterList}
import overlord.Connections.ConstantConnectionType
import overlord.Instances._

import java.nio.file.Path
import scala.collection.mutable

object OutputSoftware {
	def cpuInvariantActions(game: Game,
	                        gatePath: Path): Unit = {
		Game.pathStack.push(gatePath.toRealPath())

		val swi = game.allSoftwareInstances

		for (instance <- swi) {
			executeCpuInvariantSoftwareActions(instance, game)
		}
		Game.pathStack.pop()
	}

	private def executeCpuInvariantSoftwareActions(instance: SoftwareInstance,
	                                               game: Game): Unit = {
		val backupStack = Game.pathStack.clone()
		val connections = game.connected.toSeq

		val software = instance.definition
		val actions  = software.actionsFile.actions

		for (action <- actions.filter(_.phase == 1)) {
			val conParameters = connections
				.filter(_.isUnconnected)
				.map(_.asUnconnected)
				.filter(_.isConstant).map(c => {
				val constant = c.connectionType.asInstanceOf[ConstantConnectionType]
				val name     = c.secondFullName.split('.').lastOption match {
					case Some(value) => value
					case None        => c.secondFullName
				}
				Map[String, Variant](name -> constant.constant)
			}).fold(Map[String, Variant]())((o, n) => o ++ n)

			val instanceSpecificParameters = instance match {
				case _ => Map[String, Variant]()
			}

			val parameters = software.parameters ++
			                 conParameters ++
			                 instanceSpecificParameters

			instance.mergeAllAttributes(parameters)

			val parametersTbl = for ((k, v) <- parameters) yield k -> (() => v)

			action.execute(instance, parametersTbl, Game.pathStack.top.resolve("soft"))
		}

		Game.pathStack = backupStack
	}

	def cpuSpecificActions(game: Game,
	                       gatePath: Path): Unit = {
		Game.pathStack.push(gatePath.toRealPath())

		val swi = game.allSoftwareInstances

		for (instance <- swi) {
			executeCpuSpecificSoftwareActions(instance, game)
		}
		Game.pathStack.pop()
	}

	private def executeCpuSpecificSoftwareActions(instance: SoftwareInstance,
	                                              game: Game): Unit = {
		val backupStack = Game.pathStack.clone()
		val actions     = instance.definition.actionsFile.actions

		for (action <- actions.filter(_.phase == 2)) {
			for (cpu <- game.cpus) {
				val sanitizedTriple = cpu.triple.replace("-", "_")

				val keywords = Map[String, () => Variant](
					"${name}" -> (() => StringV(instance.name)),
					"${triple}" -> (() => StringV(sanitizedTriple)),
					"${TRIPLE}" -> (() => StringV(sanitizedTriple.toUpperCase)),
					"${cpuName}" -> (() => StringV(cpu.ident.split('.').last)),
					"${CPUNAME}" -> (() => StringV(cpu.ident.split('.').last.toUpperCase)),
					"${cpuWidth}" -> (() => IntV(cpu.width)),
					"${maxAtomicWidth}" -> (() => IntV(cpu.maxAtomicWidth)),
					"${maxBitOpTypeWidth}" -> (() => IntV(cpu.maxBitOpTypeWidth)),
					"${isSoftCore}" -> (() => BooleanV(!cpu.isHardware)),
					"${isHardCore}" -> (() => BooleanV(cpu.isHardware)),
					"${memoryMap}" -> (() => StringV(generateMemoryMapFor(game, cpu))),
					)

				val parametersTbl = keywords
				action.execute(instance, parametersTbl, Game.pathStack.top.resolve("soft"))
			}

		}
		Game.pathStack = backupStack
	}

	private def generateMemoryMapFor(game: Game, cpu: CpuInstance): String = {
		val sb = new StringBuilder

		// find all ram linked to this cpu
		val ramByBus = (for (ram <- game.getRAMConnectedTo(cpu)) yield {
			val buses = game.getEndBusesConnecting(cpu, ram)
			if (buses.isEmpty) {
				print(f"No bus between ${cpu.ident} and ${ram.ident}%n")
				None
			} else {
				var bus    : BusInstance = buses.head
				var minDist: Int         = Int.MaxValue
				var minD1  : Int         = Int.MaxValue

				for (b <- buses) {
					val d0 = game.distanceMatrix.distanceBetween(cpu, b)
					val d1 = game.distanceMatrix.distanceBetween(b, ram)
					val d  = d0 + d1

					// use shortest distance between cpu and ram,
					// if tie use bus thats closest to the ram
					if (d < minDist) {
						minDist = d
						bus = b
						minD1 = d1
					} else if (d == minDist && d1 < minD1) {
						minDist = d
						bus = b
						minD1 = d1
					}
				}
				Some(ram, bus)
			}
		}).flatten

		// do total sizes first largest first
		for ((ram, bus) <- ramByBus.sortWith((a, b) => a._1.getSizeInBytes >
		                                               b._1.getSizeInBytes)) {
			sb ++= f"%n"
			val si = ram.ident.split('.').tail.mkString("_").replace('.', '_')

			val asplits = bus.getConsumerAddressesAndSizes(ram)
			if (asplits.nonEmpty) {
				val totalSize = asplits.foldLeft(BigInt(0))((accum, a) => accum + a._2)
				sb ++= f"#define ${si.toUpperCase()}_TOTAL_SIZE_IN_BYTES " +
				       f"$totalSize${cPostFix(totalSize)}%n"
			}
		}

		// now each address range sorted by address
		val ramByRange = (for ((ram, bus) <- ramByBus) yield {
			val asplits = bus.getConsumerAddressesAndSizes(ram)
			if (asplits.isEmpty) None
			else for (((address, size), i) <- asplits
				.sortWith((a, b) => a._1 < b._1)
				.zipWithIndex) yield {
				(ram, i, address, size)
			}
		}).flatten

		for ((ram, i, address, size) <- ramByRange) {
			sb ++= f"%n"

			val si   = ram.ident.split('.').tail.mkString("_").replace('.', '_')
			val name = s"${si}_$i".toUpperCase
			sb ++= f"#define ${name}_BASE_ADDR 0x$address%x${cPostFix(size)}%n"
			sb ++= f"#define ${name}_SIZE_IN_BYTES $size${cPostFix(size)}%n"
		}

		sb ++= genCpuRegisterBanksMemoryMapFor(cpu, game)
		sb ++= genChipMemoryMapFor(cpu, game)

		sb.result()
	}

	private def cPostFix(s: BigInt): String = if (s > Int.MaxValue) "ULL" else "U"

	private def genChipMemoryMapFor(cpu: CpuInstance, game: Game): String = {
		val busStack = mutable.Stack[(BusInstance, BigInt)]()

		for (bus <- game.getDirectBusesConnectedTo(cpu))
			busStack.push((bus, bus.busBaseAddr))

		val sb = new StringBuilder
		while (busStack.nonEmpty) {
			val (bus, busAddr) = busStack.pop()

			for (instance <- bus.consumerInstances
			     if !instance.isInstanceOf[RamInstance]) {

				val asplits = bus.getConsumerAddressesAndSizes(instance)
				for ((address, size) <- asplits) {

					instance match {
						case bridge: BridgeInstance =>
							val obuses = game.getDirectBusesConnectedTo(bridge).filter(_ != bus)
							for (obus <- obuses) busStack.push((obus, address))
						case _                      =>

							for (rb <- instance.registerBanks) sb ++= writeRegisterBank(busAddr, rb)

					}
				}
			}
		}
		sb.result()
	}

	private def writeRegisterBank(busAddr: BigInt,
	                              rb: RegisterBank): String = {
		val sb    = new StringBuilder
		val name  = rb.registerListName.split('/').last
		val uname = name.toUpperCase()

		sb ++= f"// ${rb.name} 0x${busAddr + rb.address}%x ${rb.registerListName}%n"

		// -1 bank address mean non MMIO register so no base address is valid
		if (rb.address > -1) {
			sb ++= f"#define ${rb.name}_BASE_ADDR 0x${busAddr + rb.address}%x%n"
			sb ++= f"#define ${rb.name}_REGISTER(reg) ${uname}_##reg##_OFFSET%n"
		} else {
			sb ++=
			f"// These registers aren't MMIO accessible so require platform intrinsics to " +
			f"use%n"
		}
		sb ++= f"#define ${rb.name}_FIELD(reg, field) ${uname}_##reg##_##field%n"
		sb ++= f"#define ${rb.name}_FIELD_MASK(reg, field) ${uname}_##reg##_##field##_MASK%n"
		sb ++=
		f"#define ${rb.name}_FIELD_LSHIFT(reg, field) ${uname}_##reg##_##field##_LSHIFT%n"
		sb.result()
	}

	private def genCpuRegisterBanksMemoryMapFor(cpu: CpuInstance, game: Game): String = {
		val sb = new StringBuilder

		// write banks
		for (bank <- cpu.registerBanks) {
			sb ++= writeRegisterBank(0, bank)
		}

		sb.result()
	}

	def hardwareRegistersOutput(game: Game, gatePath: Path): Unit = {
		Game.pathStack.push(gatePath.toRealPath())

		for (cpu <- game.cpus) {
			genCpuHeadersFor(cpu,
			                 game,
			                 Game.pathStack.top.resolve("soft"))
			genInstanceHeadersFor(cpu,
			                      game,
			                      Game.pathStack.top.resolve("soft"))

		}
		Game.pathStack.pop()
	}

	private def genCpuHeadersFor(cpu: CpuInstance, game: Game, out: Path): Unit = {
		Utils.ensureDirectories(hwPath(cpu, out))

		// output register definitions
		for (rl <- cpu.registerLists) {
			val name  = rl.name.split('/').last
			val uname = name.toUpperCase()

			val sb = new StringBuilder
			sb ++= f"#pragma once%n"
			sb ++= f"// Copyright Deano Calver%n"
			sb ++= f"// SPDX-License-Identifier: MIT%n"
			sb ++= f"// ${rl.name}%n"
			if (rl.description.nonEmpty) sb ++= f"// ${rl.description}%n"

			sb ++= genRegisterList(s"${uname}_".toUpperCase(), rl)

			Utils.writeFile(filePath(hwPath(cpu, out), rl.name), sb.result())
		}

	}

	private def hwPath(cpu: CpuInstance, out: Path) =
		out.resolve("libraries")
			.resolve("hw")
			.resolve("include")
			.resolve("hw_regs")
			.resolve(cpu.splitIdent.last)

	private def filePath(out: Path, name: String): Path = {
		val fn       = s"${name.replace('.', '_')}.h".toLowerCase()
		val filename = Path.of(fn).getFileName
		out.resolve(filename)
	}

	private def genRegisterList(prefix: String, rl: RegisterList): String = {
		val sb = new StringBuilder
		for (reg <- rl.registers) {
			sb ++= f"%n// ${reg.desc}%n"
			sb ++= f"#define $prefix${reg.name}_OFFSET ${reg.offset}%#010xU%n"

			var totalUserMask     : Long = 0
			var reservedFieldCount: Long = -1 // we preincrement so start at -1 to get 0

			reg.fields.foreach(f => {
				sb ++= f"//${
					if (f.shortDesc.nonEmpty) f.shortDesc.get
					else ""
				}%n"

				val bits = BitsDesc(f.bits)
				totalUserMask |= (if (f.name != "RESERVED") bits.mask else 0)

				val fieldName = if (f.name == "RESERVED") {
					reservedFieldCount += 1
					s"RESERVED$reservedFieldCount"
				}
				else f.name

				val name = reg.name.toUpperCase()

				sb ++= f"#define $prefix${name}_${fieldName}_LSHIFT ${bits.lo}%#010xU%n"
				sb ++= f"#define $prefix${name}_${fieldName}_MASK ${bits.mask}%#010xU%n"
				if (bits.singleBit)
					sb ++= f"#define $prefix${name}_$fieldName ${bits.mask}%#10xU%n"

				for (elem <- f.enums) {
					if (elem.description.isDefined)
						sb ++= f"// ${elem.description}%n"
					sb ++= f"#define $prefix${name}_${fieldName}_${elem.name.toUpperCase()} ${
						elem.value
							.toString()
					}%n"
				}
			})
			if (totalUserMask != 0)
				sb ++=
				f"#define $prefix${reg.name}_USERMASK ${totalUserMask & 0xFFFFFFFF}%#010xU%n"

		}

		sb.result()
	}

	private def genInstanceHeadersFor(cpu: CpuInstance, game: Game, out: Path): Unit = {
		val definitionsWritten = mutable.HashMap[String, String]()

		val directBuses = game.getDirectBusesConnectedTo(cpu)
		val busStack    = mutable.Stack[(BusInstance, BigInt)]()
		for (bus <- directBuses) busStack.push((bus, bus.busBaseAddr))

		while (busStack.nonEmpty) {
			val (bus, busAddr) = busStack.pop()

			for (instance <- bus.consumerInstances
			     if !instance.isInstanceOf[RamInstance]) {
				val asplits = bus.getConsumerAddressesAndSizes(instance)
				for ((address, size) <- asplits) {

					instance match {
						case bridge: BridgeInstance =>
							val obuses = game.getDirectBusesConnectedTo(bridge).filter(_ != bus)
							for (obus <- obuses) busStack.push((obus, address))
						case _                      =>
							val rbs = instance.registerBanks
							val rls = instance.registerLists

							for (rb <- rbs) {
								val name  = rb.registerListName.split('/').last
								val uname = name.toUpperCase()
								val rlo   = rls.find(_.name == rb.registerListName)
								if (rlo.isEmpty) println(s"${cpu.ident} $rb.registerListName} not found")
								else if (!definitionsWritten.contains(rb.registerListName)) {
									rls.find(_.name == rb.registerListName) match {
										case Some(rl) =>
											if (!definitionsWritten.contains(rb.registerListName)) {
												val sbrl = new StringBuilder
												sbrl ++= f"#pragma once%n"
												sbrl ++= f"// Copyright Deano Calver%n"
												sbrl ++= f"// SPDX-License-Identifier: MIT%n"
												sbrl ++= f"// $name%n"
												if (rl.description.nonEmpty) sbrl ++= f"// ${rl.description}%n"
												sbrl ++= genRegisterList(s"${uname}_", rl)
												Utils.writeFile(filePath(hwRegsPath(cpu, out), name),
												                sbrl.result())

												definitionsWritten += (rb.registerListName -> name)
											}
										case None     =>
											println(f"Cannot find ${rb.registerListName} register list%n")
									}
								}
							}
					}
				}
			}
		}
	}

	private def hwRegsPath(cpu: CpuInstance, out: Path) =
		out.resolve("libraries")
			.resolve("hw")
			.resolve("include")
			.resolve("hw_regs")


}

