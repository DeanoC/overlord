package overlord

import ikuy_utils._
import overlord.Chip.{BitsDesc, RegisterBank, RegisterList, Registers}
import overlord.Connections._
import overlord.Instances._
import overlord.Interfaces.{RamLike, RegisterBankLike}

import java.nio.file.Path
import scala.collection.mutable

object OutputSoftware {
	def cpuInvariantActions(software: Seq[SoftwareInstance],
	                        connected: Seq[Connected]): Unit = {
		for (instance <- software) executeCpuInvariantSoftwareActions(instance, connected)
	}

	private def executeCpuInvariantSoftwareActions(instance: SoftwareInstance,
	                                               connected: Seq[Connected]): Unit = {
		val software = instance.definition
		val actions  = software.actionsFile.actions

		for (action <- actions.filter(_.phase == 1)) {
			val conParameters = connected
				.filter(_.isInstanceOf[ConnectedConstant])
				.map(_.asInstanceOf[ConnectedConstant])
				.map(c => {
					val name = c.secondFullName.split('.').lastOption match {
						case Some(value) => value
						case None        => c.secondFullName
					}
					Map[String, Variant](name -> c.constant)
				}).fold(Map[String, Variant]())((o, n) => o ++ n)

			val instanceSpecificParameters = instance match {
				case _ => Map[String, Variant]()
			}

			val parameters = software.parameters ++ conParameters ++ instanceSpecificParameters

			instance.mergeAllAttributes(parameters)

			val parametersTbl = for ((k, v) <- parameters) yield k -> v

			action.execute(instance, parametersTbl)
		}

	}

	def cpuSpecificActions(boardName: String,
	                       software: Seq[SoftwareInstance],
	                       cpus: Seq[CpuInstance],
	                       distanceMatrix: DistanceMatrix,
	                       connected: Seq[Connected]): Unit = {
		val keywordsPerCpu: Array[Map[String, Variant]] = (for (cpu <- cpus) yield {
			val sanitizedTriple           = cpu.triple.replace("-", "_")
			val (memMap, registerHelpers) = generateMemoryMapFor(cpu, distanceMatrix, connected)
			Map[String, Variant](
				"${triple}" -> StringV(sanitizedTriple),
				"${TRIPLE}" -> StringV(sanitizedTriple.toUpperCase),
				"${cpuName}" -> StringV(cpu.cpuType),
				"${CPUNAME}" -> StringV(cpu.cpuType.toUpperCase),
				"${cpuWidth}" -> IntV(cpu.width),
				"${cpuCoreCount}" -> IntV(cpu.cpuCount),
				"${maxAtomicWidth}" -> IntV(cpu.maxAtomicWidth),
				"${maxBitOpTypeWidth}" -> IntV(cpu.maxBitOpTypeWidth),
				"${isSoftCore}" -> BooleanV(!cpu.isHardware),
				"${isHardCore}" -> BooleanV(cpu.isHardware),
				"${memoryMap}" -> StringV(memMap),
				"${registerHelpers}" -> StringV(registerHelpers),
				"${board}" -> StringV(boardName)
				)
		}).toArray

		for (instance <- software) executeCpuSpecificSoftwareActions(instance, cpus, keywordsPerCpu)
	}

	private def executeCpuSpecificSoftwareActions(instance: SoftwareInstance,
	                                              cpus: Seq[CpuInstance],
	                                              keywordsPerCpu: Array[Map[String, Variant]]): Unit = {
		val actions = instance.definition.actionsFile.actions

		for (action <- actions.filter(_.phase == 2)) {
			for (iCpu <- cpus.indices) action.execute(instance, keywordsPerCpu(iCpu) + ("${name}" -> StringV(instance.name)))
		}
	}

	private def generateMemoryMapFor(cpu: CpuInstance,
	                                 distanceMatrix: DistanceMatrix,
	                                 connected: Seq[Connected]): (String, String) = {
		val mmsb = new mutable.StringBuilder
		val hesb = new mutable.StringBuilder

		val (mm, helpers) = genChipMemoryMapFor(cpu, distanceMatrix, connected)
		mmsb ++= mm
		hesb ++= helpers

		mmsb ++=
		"""
			|#include "register_helpers.h"
			|""".stripMargin

		(mmsb.result(), hesb.result())
	}

	private def cPostFix(s: BigInt): String = if (s > Int.MaxValue) "ULL" else "U"

	private def genChipMemoryMapFor(cpu: CpuInstance, distanceMatrix: DistanceMatrix, connected: Seq[Connected]): (String, String) = {

		val (ramRanges, chipAddresses) = extractRegisters(cpu, distanceMatrix, connected)

		// add cpu specific register banks
		for (rb <- cpu.banks) if (rb.baseAddress != -1) chipAddresses += ((rb.name, cpu, rb.baseAddress))

		val ramSizesByRam = mutable.Map[String, BigInt]()
		for ((ram, _, _, size) <- ramRanges) {
			val si = ram.getOwner.name.replace('.', '_')
			if (ramSizesByRam.contains(si)) ramSizesByRam(si) += size
			else ramSizesByRam += (si -> size)
		}

		val rsb = new mutable.StringBuilder
		for ((name, size) <- ramSizesByRam) rsb ++= f"%n#define ${name.toUpperCase}_TOTAL_SIZE_IN_BYTES $size${cPostFix(size)}%n"

		for ((ram, i, address, size) <- ramRanges) {
			rsb ++= f"%n"
			val si   = ram.getOwner.name.replace('.', '_')
			val name = s"${si}_$i".toUpperCase
			rsb ++= f"#define ${name}_BASE_ADDR 0x$address%x${cPostFix(address)}%n"
			rsb ++= f"#define ${name}_SIZE_IN_BYTES $size${cPostFix(size)}%n"
		}

		val hesb = new mutable.StringBuilder
		for ((name, _, address) <- chipAddresses) {
			val nameU = s"${name}".toUpperCase
			rsb ++= f"%n#define ${nameU}_BASE_ADDR 0x$address%x${cPostFix(address)}%n"
			hesb ++= f"#define ${name}_FIELD(reg, field) ${nameU}_##reg##_##field%n"
			hesb ++= f"#define ${name}_FIELD_MASK(reg, field) ${nameU}_##reg##_##field##_MASK%n"
			hesb ++= f"#define ${name}_FIELD_LSHIFT(reg, field) ${nameU}_##reg##_##field##_LSHIFT%n"
			hesb ++= f"#define ${name}_FIELD_ENUM(reg, field, enm) ${nameU}_##reg##_##field##_##enm%n"

		}

		(rsb.result(), hesb.result())


		/*
	val busStack: mutable.Stack[(BusLike, BigInt)] = {
		val builder = mutable.Stack.newBuilder[(BusLike, BigInt)]
		game.getInterfacesDirectlyConnectedTo[BusLike](cpu)
			.filter(inf => game.connected.exists(p => p.connectedTo(inf.getOwner)))
			.foreach(bus => builder += ((bus, bus.getBaseAddress)))
		builder.result()
	}

	while (busStack.nonEmpty) {
		val (bus, busAddr) = busStack.pop()

		for (instance <- bus.consumerInstances) {
			val asplits = bus.getConsumerAddressesAndSizes(instance)
			for ((address, size) <- asplits) {

				instance match {
					case bridge: BridgeInstance =>
						val obuses = game.getDirectBusesConnectedTo(bridge).filter(_ != bus)
						for (obus <- obuses) busStack.push((obus, address))
					case _                      =>
						if (instance.registerBanks.isEmpty && instance.registerLists.nonEmpty) {
							sb ++= writeRegisterBank(busAddr,
																			 RegisterBank(instance.ident.toUpperCase,
																										address,
																										instance.registerLists(0).name))
						} else {
							for (rb <- instance.registerBanks) sb ++= writeRegisterBank(busAddr, rb)
						}

				}
			}
		}
	}*/

	}

	private def extractRegisters(cpu: CpuInstance, distanceMatrix: DistanceMatrix, connected: Seq[Connected]) = {

		val ramRanges     = mutable.ArrayBuffer[(RamLike, Int, BigInt, BigInt)]()
		val chipAddresses = mutable.ArrayBuffer[(String, ChipInstance, BigInt)]()

		distanceMatrix.connectedTo(cpu).filter(_.isVisibleToSoftware).foreach(ep => {
			// is possible to have several routes between chips (multiple buses) by having multiple connects with different buses
			// currently we ignore any but the first (TODO trace the correct route)
			val routeConnections: Seq[Connected] = distanceMatrix.expandedRouteBetween(cpu, ep).map {
				case (src, dst) => {
					val cons = connected.filter(con => con.connectedBetween(src, dst, FirstToSecondConnection()))
					val hd   = cons.head
					// try an swap port group for a parallel bus connection which has info about which bus was used
					if (hd.isInstanceOf[ConnectedPortGroup]) {
						val found = connected.find(p => p.isInstanceOf[ConnectedBus] &&
						                                p.firstFullName == hd.firstFullName &&
						                                p.secondFullName == hd.secondFullName)

						if (found.nonEmpty) found.get
						else cons.head
					} else cons.head
				}
			}

			var address: BigInt = 0
			routeConnections.foreach { r =>
				r match {
					case bus: ConnectedBus => address += bus.bus.getBaseAddress
					case _                 =>
				}
				// only do chips once per cpu
				if (r.second.nonEmpty) {
					if (r.second.get.instance.hasInterface[RamLike]) {
						val ram = r.second.get.instance.getInterfaceUnwrapped[RamLike]
						for (((addr, size), index) <- ram.getRanges.zipWithIndex) ramRanges += ((ram, index, address + addr, size))
					}

					val other = r.second.get.instance.asInstanceOf[ChipInstance]
					if ((!chipAddresses.exists { case (_, o, _) => (o == other) }) &&
					    other.isVisibleToSoftware &&
					    other.hasInterface[RegisterBankLike]) {
						val registerBank = other.getInterfaceUnwrapped[RegisterBankLike]
						if (other.instanceNumber >= registerBank.maxInstances)
							println(s"${other.name}: not enough instances\n")
						else {
							if (registerBank.banks.isEmpty) chipAddresses += ((other.name, other, address))
							registerBank.banks.foreach { rb =>
								if (rb.baseAddress != -1) {
									val rbName = if (rb.name.isEmpty) r.secondFullName
									else rb.name.replace("""${index}""", other.instanceNumber.toString)
									chipAddresses += ((rbName,
										other,
										address + rb.baseAddress + (rb.addressIncrement * other.instanceNumber)))
								}
							}
						}
					}
				}
			}
		})
		(ramRanges, chipAddresses)
	}

	def hardwareRegistersOutput(cpus: Seq[CpuInstance], distanceMatrix: DistanceMatrix, connected: Seq[Connected]): Unit = {
		for (cpu <- cpus) {
			Utils.ensureDirectories(hwPath(cpu, Game.outPath))

			val (ramRanges, chipAddresses) = extractRegisters(cpu, distanceMatrix, connected)

			var allRegLists = (for ((_, chip, _) <- chipAddresses; bank <- chip.banks) yield bank.registerListName) ++ cpu.banks.map(f => f.registerListName)

			val uniqueRegisterLists = allRegLists.toSet.toSeq

			for (rl <- uniqueRegisterLists) genCpuHeadersFor(cpu, rl)

			genInstanceHeadersFor(cpu, Game.outPath)
		}
	}

	private def genCpuHeadersFor(cpu: CpuInstance, registerListName: String): Unit = {
		// output register definitions
		val rl    = Registers.registerListCache(registerListName)
		val name  = rl.name.split('/').last.replace(".toml", "")
		val uname = name.toUpperCase()

		val sb = new mutable.StringBuilder
		sb ++= f"#pragma once%n"
		sb ++= f"// Copyright Deano Calver%n"
		sb ++= f"// SPDX-License-Identifier: MIT%n"
		sb ++= f"// ${rl.name}%n"
		if (rl.description.nonEmpty) sb ++= f"// ${rl.description}%n"

		sb ++= genRegisterList(s"${uname}_".toUpperCase(), rl)

		Utils.writeFile(filePath(hwPath(cpu, Game.outPath), rl.name), sb.result())
	}

	private def hwPath(cpu: CpuInstance, out: Path) = hwRegsPath(cpu, out).resolve(cpu.cpuType).resolve("registers")

	private def hwRegsPath(cpu: CpuInstance, out: Path) =
		out.resolve("libs")
			.resolve("platform")
			.resolve("include")
			.resolve("platform")

	private def genRegisterList(prefix: String, rl: RegisterList): String = {
		val sb = new mutable.StringBuilder
		for (reg <- rl.registers) {
			sb ++= f"%n// ${reg.desc}%n"
			sb ++= f"#define $prefix${reg.name}_OFFSET ${reg.offset}%#010xU%n"

			var totalUserMask     : Long = 0
			var reservedFieldCount: Long = -1 // we preincrement so start at -1 to get 0

			reg.fields.foreach(f => {
				sb ++= f"//${if (f.shortDesc.nonEmpty) f.shortDesc.get else ""}%n"

				val bits = BitsDesc(f.bits)
				totalUserMask |= (if (f.name != "RESERVED") bits.mask else 0)

				val fieldName = if (f.name == "RESERVED") {
					reservedFieldCount += 1
					s"RESERVED$reservedFieldCount"
				}
				else f.name

				val name = reg.name.toUpperCase().replace('.', '_')

				sb ++= f"#define $prefix${name}_${fieldName}_LSHIFT ${bits.lo}%#010xU%n"
				sb ++= f"#define $prefix${name}_${fieldName}_MASK ${bits.mask}%#010xU%n"
				if (bits.singleBit) sb ++= f"#define $prefix${name}_$fieldName ${bits.mask}%#10xU%n"

				for (elem <- f.enums) {
					if (elem.description.isDefined) sb ++= f"// ${elem.description.get.toString}%n"
					sb ++= f"#define $prefix${name}_${fieldName}_${elem.name.toUpperCase} ${elem.value.toString}%n"
				}
			})

			if (totalUserMask != 0) sb ++= f"#define $prefix${reg.name}_USERMASK ${totalUserMask & 0xFFFFFFFF}%#010xU%n"
		}

		sb.result()
	}

	private def genInstanceHeadersFor(cpu: CpuInstance, out: Path): Unit = {
		val definitionsWritten = mutable.HashMap[String, String]()
		/*
				val directBuses = game.getDirectBusesConnectedTo(cpu)
				val busStack    = mutable.Stack[(BusInstance, BigInt)]()
				for (bus <- directBuses) busStack.push((bus, bus.busBaseAddr))

				while (busStack.nonEmpty) {
					val (bus, busAddr) = busStack.pop()

					for (instance <- bus.consumerInstances) {
						val asplits = bus.getConsumerAddressesAndSizes(instance)
						for ((address, size) <- asplits) {

							instance match {
								case bridge: BridgeInstance =>
									val obuses = game.getDirectBusesConnectedTo(bridge).filter(_ != bus)
									for (obus <- obuses) busStack.push((obus, address))
								case _                      =>
									val rbs = if (instance.registerBanks.isEmpty &&
																instance.registerLists.nonEmpty) {
										for (rl <- instance.registerLists) yield
											RegisterBank(instance.ident.toUpperCase, address, rl.name)
									} else instance.registerBanks
									val rls = instance.registerLists ++ {
										if (instance.definition.registers.isEmpty) Seq()
										else instance.definition.registers.get.lists
									}

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
				}*/
	}

	private def filePath(out: Path, name: String): Path = {
		val fn       = s"${name.replace('.', '_')}.h".toLowerCase()
		val filename = Path.of(fn).getFileName
		out.resolve(filename)
	}

	private def writeRegisterBank(busAddr: BigInt,
	                              rb: RegisterBank): (String, String) = {
		val sb    = new mutable.StringBuilder
		val mmsb  = new mutable.StringBuilder
		val name  = rb.name.replace('.', '_')
		val uname = name.toUpperCase()

		/*	val uname = rb.registerListName.split('/').last.toUpperCase().replace('.', '_')

			sb ++= f"// ${name} 0x${busAddr + rb.address}%x ${rb.registerListName}%n"
	*/
		// -1 bank address mean non MMIO register so no base address is valid
		if (rb.baseAddress > -1) {
			//			mmsb ++= f"#define ${name}_BASE_ADDR 0x${busAddr + rb.address}%x%n"
			sb ++= f"#define ${name}_REGISTER(reg) ${uname}_##reg##_OFFSET%n"
		} else {
			sb ++= f"// These registers aren't MMIO accessible so require platform intrinsics to use%n"
		}
		sb ++= f"#define ${name}_FIELD(reg, field) ${uname}_##reg##_##field%n"
		sb ++= f"#define ${name}_FIELD_MASK(reg, field) ${uname}_##reg##_##field##_MASK%n"
		sb ++= f"#define ${name}_FIELD_LSHIFT(reg, field) ${uname}_##reg##_##field##_LSHIFT%n"
		sb ++= f"#define ${name}_FIELD_ENUM(reg, field, enm) ${uname}_##reg##_##field##_##enm%n"
		(mmsb.result(), sb.result())
	}

	private def genCpuRegisterBanksMemoryMapFor(cpu: CpuInstance, game: Game): (String, String) = {
		val mmsb = new mutable.StringBuilder
		val sb   = new mutable.StringBuilder

		// write banks
		//		for (bank <- cpu.banks) {
		//			val (rbmm, rb) = writeRegisterBank(0, bank)
		//			mmsb ++= rbmm
		//			sb ++= rb
		//		}

		(mmsb.result(), sb.result())
	}

}

