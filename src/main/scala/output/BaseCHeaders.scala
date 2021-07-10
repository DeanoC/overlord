package output

import ikuy_utils.Utils
import overlord.Game
import overlord.Gateware.BitsDesc
import overlord.Instances.{BridgeInstance, BusInstance, CpuInstance}
import overlord.Software.RegisterList

import java.nio.file.Path
import java.util.Calendar
import scala.collection.mutable

object BaseCHeaders {
	def apply(game: Game, out: Path): Unit = {
		if (game.cpus.isEmpty) return

		println(s"Creating Base Headers at $out")
		Utils.ensureDirectories(out)

		genCpuHeaders(game, out)
	}
	private def genCpuHeaders(game: Game, out: Path): Unit = {
		val dT = Calendar.getInstance()

		for (cpu <- game.cpus) {
			val sb = new StringBuilder
			sb ++= f"#pragma once%n"
			sb ++= f"// Copyright Deano Calver%n"
			sb ++= f"// SPDX-License-Identifier: MIT%n"
			sb ++= f"// ${cpu.width} bit ${cpu.triple} CPU%n"
			sb ++= f"// Auto-generated on ${dT.getTime}%n"

			sb ++= f"#define CPU_BIT_WIDTH ${cpu.width}%n"
			// scala 2.16.0 has a bug related to escaping " workaround below
			sb ++= f"#define CPU_TRIPLE_STRING " +
			       '"' + s"${cpu.triple}" + '"' + f"%n"

			cpu.definition.gateware match {
				case Some(value) =>
					sb ++= f"#define CPU_IS_SOFT_CORE 1%n"
					sb ++= f"#define CPU_IS_HARD_CORE 0%n"
				case None        =>
					sb ++= f"#define CPU_IS_SOFT_CORE 0%n"
					sb ++= f"#define CPU_IS_HARD_CORE 1%n"
			}

			val path = out.resolve(s"${cpu.sanitizedTriple}")
			Utils.ensureDirectories(path)
			Utils.writeFile(path.resolve(s"platform.h"), sb.result())

			genInstanceHeadersFor(cpu, game, out)
		}
	}

	private def genInstanceHeadersFor(cpu: CpuInstance, game: Game, out: Path): Unit = {
		val dT = Calendar.getInstance()
		val definitionsWritten = mutable.HashMap[String, String]()
		val directBuses = game.getDirectBusesConnectedTo(cpu)
		val busStack = mutable.Stack[(BusInstance, BigInt)]()
		for(bus <- directBuses) busStack.push((bus, bus.busBaseAddr))

		while(busStack.nonEmpty) {
			val (bus, busAddr) = busStack.pop()

			for (instance <- bus.consumerInstances) {
				val sb = new StringBuilder
				sb ++= f"#pragma once%n"
				sb ++= f"// Copyright Deano Calver%n"
				sb ++= f"// SPDX-License-Identifier: MIT%n"
				sb ++= f"// Auto-generated on ${dT.getTime}%n"

				val (address, size) = bus.getConsumerAddressAndSize(instance)
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
							sb ++=f"// ${rb.name} 0x${busAddr +rb.address}%x ${rb.registerListName}%n"
							sb ++=f"#define ${rb.name}_BASE_ADDR 0x${busAddr +rb.address}%x%n"

							rls.find(_.name == rb.registerListName) match {
								case Some(v) =>
									if(!definitionsWritten.contains(rb.registerListName)) {
										sb ++= genRegisterList(s"${rb.name}_", v)
										definitionsWritten += (rb.registerListName -> rb.name)
									}
								case None        =>
									println(f"Cannot find ${rb.registerListName} register list%n")
							}
						}
				}
				val path = out.resolve(s"${cpu.sanitizedTriple}")
				Utils.ensureDirectories(path)
				Utils.writeFile(path.resolve(s"${instance.ident}.h"), sb.result())
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

				sb ++= f"#define $prefix${reg.name}_${fieldName}_LSHIFT ${bits.lo}%#010xU%n"
				sb ++= f"#define $prefix${reg.name}_${fieldName}_MASK ${bits.mask}%#010xU%n"
				if(bits.singleBit)
					sb++= f"#define $prefix${reg.name}_$fieldName ${bits.mask}%#10xU%n"
			})
			if(totalUserMask != 0)
				sb ++= f"#define $prefix${reg.name}_USERMASK ${totalUserMask & 0xFFFFFFFF}%#010xU%n"

		}

		sb.result()
	}

}
