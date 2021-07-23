package output

import ikuy_utils.Utils
import overlord.Game
import overlord.Gateware.BitsDesc
import overlord.Instances.{BridgeInstance, BusInstance, CpuInstance, RamInstance}
import overlord.Software.RegisterList

import java.nio.file.Path
import scala.collection.mutable

object BaseCHeaders {
	def apply(game: Game, out: Path): Unit = {
		if (game.cpus.isEmpty) return

		println(s"Creating Base Headers at $out")
		Utils.ensureDirectories(out)

		genCpuHeaders(game, out)
	}
	private def hwPath(cpu: CpuInstance, out: Path) =
		out.resolve(s"${cpu.sanitizedTriple}").resolve("hw")

	private def genCpuHeaders(game: Game, out: Path): Unit = {
		val postfix = (s:BigInt) => if(s > Int.MaxValue) "ULL" else "U"

		for (cpu <- game.cpus) {
			val sb = new StringBuilder
			sb ++= f"#pragma once%n"
			sb ++= f"// Copyright Deano Calver%n"
			sb ++= f"// SPDX-License-Identifier: MIT%n"
			sb ++= f"// ${cpu.width} bit ${cpu.triple} CPU%n"
			sb ++= f"%n"

			sb ++= f"#define CPU_BIT_WIDTH ${cpu.width}%n"
			// scala 2.16.0 has a bug related to escaping " workaround below
			sb ++= f"#define CPU_TRIPLE_STRING " +
			       '"' + s"${cpu.triple}" + '"' + f"%n"
			sb ++= f"%n"

			cpu.definition.gateware match {
				case Some(value) =>
					sb ++= f"#define CPU_IS_SOFT_CORE 1%n"
					sb ++= f"#define CPU_IS_HARD_CORE 0%n"
				case None        =>
					sb ++= f"#define CPU_IS_SOFT_CORE 0%n"
					sb ++= f"#define CPU_IS_HARD_CORE 1%n"
			}

			// find all ram linked to this cpu
			for( ram <- game.getRAMConnectedTo(cpu)) {
				sb ++= f"%n"
				val buses = game.getEndBusesConnecting(cpu, ram)

				if(buses.isEmpty) printf(f"No bus between ${cpu.ident} and ${ram.ident}%n")
				else {
					var bus: BusInstance = buses.head
					var minDist: Int = Int.MaxValue
					for(b <- buses) {
						val d = game.distanceMatrix.distanceBetween(cpu, b) +
						        game.distanceMatrix.distanceBetween(b, ram)

						if(d < minDist) {
							minDist = d
							bus = b
						}
					}

					val si = ram.ident.split('.').tail.mkString("_").replace('.', '_')
					val totalSize = ram.getSizeInBytes
					sb ++= f"#define ${si.toUpperCase()}_TOTAL_SIZE_IN_BYTES $totalSize${postfix(totalSize)}%n"

					val asplits = bus.getConsumerAddressesAndSizes(ram)
					var i = 0
					for ((address, size) <- asplits) {
						val pf = if(i != 0)s"_$i" else ""
						val name         = s"$si$pf".toUpperCase

						sb ++= f"#define ${name}_BASE_ADDR 0x$address%x${postfix(size)}%n"
						sb ++= f"#define ${name}_SIZE_IN_BYTES $size${postfix(size)}%n"
						i = i + 1
					}
				}
			}

			val path = hwPath(cpu, out)
			Utils.ensureDirectories(path)
			Utils.writeFile(path.resolve(s"platform.h"), sb.result())

			genCpuHeadersFor(cpu, game, out)
			genInstanceHeadersFor(cpu, game, out)
		}
	}
	private def genCpuHeadersFor(cpu: CpuInstance, game: Game, out: Path): Unit = {
		val definitionsWritten = mutable.HashMap[String, String]()

		// output register definations first
		for(rl <- cpu.registerLists) {
			val sb = new StringBuilder
			sb ++= f"#pragma once%n"
			sb ++= f"// Copyright Deano Calver%n"
			sb ++= f"// SPDX-License-Identifier: MIT%n"
			sb ++= f"// ${rl.name}%n"
			if(rl.description.nonEmpty) sb ++= f"// ${rl.description}%n"

			sb ++= genRegisterList(s"${rl.name}_".toUpperCase(), rl)

			val path = hwPath(cpu, out)
			Utils.ensureDirectories(path)
			val fn = s"${rl.name.replace('.', '_')}.h".toLowerCase()
			Utils.writeFile(path.resolve(fn), sb.result())
		}

		// now write banks
		for(bank <- cpu.registerBanks) {
			val rlo = cpu.registerLists.find(_.name == bank.registerListName)

			if(rlo.isEmpty) println(s"${cpu.ident} ${bank.registerListName} not found")
			else if (!definitionsWritten.contains(bank.registerListName)) {
				val rl = rlo.get
				val sb = new StringBuilder
				sb ++= f"#pragma once%n"
				sb ++= f"// Copyright Deano Calver%n"
				sb ++= f"// SPDX-License-Identifier: MIT%n"
				sb ++= f"// ${bank.name}%n"
				if(rl.description.nonEmpty) sb ++= f"// ${rl.description}%n"

				val rlfn = s"${rl.name.replace('.', '_')}.h".toLowerCase()
				sb ++= ("#include " + '"' + rlfn + '"' + f"%n")

				sb ++= f"#define ${bank.name}_BASE_ADDR 0x${bank.address}%x%n"

				val path = hwPath(cpu, out)
				Utils.ensureDirectories(path)
				val fn = s"${bank.name.replace('.', '_')}.h".toLowerCase()
				Utils.writeFile(path.resolve(fn), sb.result())
			}
		}
	}

	private def genInstanceHeadersFor(cpu: CpuInstance, game: Game, out: Path): Unit = {
		val definitionsWritten = mutable.HashMap[String, String]()
		val directBuses = game.getDirectBusesConnectedTo(cpu)
		val busStack = mutable.Stack[(BusInstance, BigInt)]()
		for(bus <- directBuses) busStack.push((bus, bus.busBaseAddr))

		while(busStack.nonEmpty) {
			val (bus, busAddr) = busStack.pop()

			for (instance <- bus.consumerInstances
			     if !instance.isInstanceOf[RamInstance]) {
				val sb = new StringBuilder
				sb ++= f"#pragma once%n"
				sb ++= f"// Copyright Deano Calver%n"
				sb ++= f"// SPDX-License-Identifier: MIT%n"

				val asplits = bus.getConsumerAddressesAndSizes(instance)
				for((address, size) <- asplits) {
					sb ++= f"// ${instance.ident} @ 0x$address%x size $size%n"

					instance match {
						case bridge: BridgeInstance =>
							val obuses = game.getDirectBusesConnectedTo(bridge).filter(_ != bus)
							for (obus <- obuses) busStack.push((obus, address))
						case _                      =>
							sb ++= f"// ${instance.ident} has ${instance.registerBanks.length} banks%n"
							val rbs = instance.registerBanks
							val rls = instance.registerLists

							for (rb <- rbs) {
								sb ++=
								f"// ${rb.name} 0x${busAddr + rb.address}%x ${rb.registerListName}%n"
								sb ++= f"#define ${rb.name}_BASE_ADDR 0x${busAddr + rb.address}%x%n"

								rls.find(_.name == rb.registerListName) match {
									case Some(v) =>
										if (!definitionsWritten.contains(rb.registerListName)) {
											sb ++= genRegisterList(s"${rb.name}_".toUpperCase, v)
											definitionsWritten += (rb.registerListName -> rb.name)
										}
									case None    =>
										println(f"Cannot find ${rb.registerListName} register list%n")
								}
							}
					}
				}
				instance match{
					case _:BusInstance =>
					case _:BridgeInstance =>
					case _ =>
						val path = hwPath(cpu, out)
						Utils.ensureDirectories(path)
						val fn = s"${instance.ident.replace('.', '_')}.h".toLowerCase()
						Utils.writeFile(path.resolve(fn), sb.result())
				}
			}
		}
	}

	private def genRegisterList(prefix: String, rl : RegisterList): String = {
		val sb = new StringBuilder
		for(reg <- rl.registers) {
			sb ++= f"%n// ${reg.desc}%n"
			sb ++= f"#define $prefix${reg.name}_OFFSET ${reg.offset}%#010xU%n"

			var totalUserMask : Long = 0
			var reservedFieldCount : Long = -1 // we preincrement so start at -1 to get 0

			reg.fields.foreach( f => {
				sb ++= f"//${
				if(f.shortDesc.nonEmpty) f.shortDesc.get
				else ""}%n"

				val bits = BitsDesc(f.bits)
				totalUserMask |= (if(f.name != "RESERVED") bits.mask else 0)

				val fieldName = if(f.name == "RESERVED") {
					reservedFieldCount+=1
					s"RESERVED$reservedFieldCount"
				}
				else f.name

				val name = reg.name.toUpperCase()

				sb ++= f"#define $prefix${name}_${fieldName}_LSHIFT ${bits.lo}%#010xU%n"
				sb ++= f"#define $prefix${name}_${fieldName}_MASK ${bits.mask}%#010xU%n"
				if(bits.singleBit)
					sb++= f"#define $prefix${name}_$fieldName ${bits.mask}%#10xU%n"
			})
			if(totalUserMask != 0)
				sb ++= f"#define $prefix${reg.name}_USERMASK ${totalUserMask & 0xFFFFFFFF}%#010xU%n"

		}

		sb.result()
	}

}
