package overlord

import ikuy_utils._
import overlord.Chip.{BitsDesc, RegisterList, Registers}
import overlord.Connections._
import overlord.Instances._
import overlord.Interfaces.{RamLike, RegisterBankLike}

import java.nio.file.Path
import scala.collection.mutable

object OutputSoftware {
	def cpuInvariantActions(software: Seq[SoftwareInstance],
	                        constants: Seq[Constant]): Unit = {
		for (instance <- software) executeCpuInvariantSoftwareActions(instance, constants)
	}

	private def executeCpuInvariantSoftwareActions(instance: SoftwareInstance,
	                                               constants: Seq[Constant]): Unit = {
		val software = instance.definition
		val actions  = software.actionsFile.actions

		for (action <- actions.filter(_.phase == 1)) {
			val conParameters = constants.collect { case cc: Constant => cc }
				.map(c => {
					val name = if (c.parameter.name.isEmpty) c.instance.name else c.parameter.name
					c.parameter.parameterType match {
						case ConstantParameterType(value) => Map[String, Variant](name -> value)
						case FrequencyParameterType(freq) => Map[String, Variant](name -> DoubleV(freq))
					}
				}).fold(Map[String, Variant]())((o, n) => o ++ n)

			val instanceSpecificParameters = instance match {
				case _ => Map[String, Variant]()
			}

			val parameters = software.parameters ++ conParameters ++ instanceSpecificParameters

			instance.mergeAllAttributes(parameters)

			instance.finalParameterTable.addAll(for ((k, v) <- parameters) yield k -> v)

			action.execute(instance, instance.finalParameterTable.toMap)
		}

	}

	def cpuSpecificActions(boardName: String,
	                       software: Seq[SoftwareInstance],
	                       cpus: Seq[CpuInstance],
	                       distanceMatrix: DistanceMatrix,
	                       connected: Seq[Connected]): Unit = {
		val keywordsPerCpu: Array[Map[String, Variant]] = (for (cpu <- cpus) yield {
			val sanitizedTriple = cpu.triple.replace("-", "_")
			val memMap          = generateMemoryMapFor(cpu, distanceMatrix, connected)
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
			for (iCpu <- cpus.indices) action.execute(instance, keywordsPerCpu(iCpu) + ("${name}" -> StringV(instance.name)) ++ instance.finalParameterTable.toMap)
		}
	}

	private def generateMemoryMapFor(cpu: CpuInstance,
	                                 distanceMatrix: DistanceMatrix,
	                                 connected: Seq[Connected]): String = {
		val mmsb = new mutable.StringBuilder

		val mm = genChipMemoryMapFor(cpu, distanceMatrix, connected)
		mmsb ++= mm


		mmsb.result()
	}

	private def genChipMemoryMapFor(cpu: CpuInstance, distanceMatrix: DistanceMatrix, connected: Seq[Connected]): String = {

		val (ramRanges, chipAddresses) = extractRamAndRegisters(cpu, distanceMatrix, connected)

		// add cpu specific register banks
		for (rb <- cpu.banks) if (rb.baseAddress != -1) chipAddresses += ((rb.name, rb.name, cpu, rb.baseAddress))

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

		for ((rbName, rlName, _, address) <- chipAddresses) {
			val rbNameU = rbName.toUpperCase
			val rlNameU = rlName.toUpperCase

			if (address >= 0) {
				rsb ++= f"%n#define ${rbNameU}_BASE_ADDR 0x$address%x${cPostFix(address)}%n"
			} else {
				// -1 bank address mean non MMIO register so no base address is valid
				rsb ++= f"// These registers aren't MMIO accessible so require platform intrinsics to use%n"
			}
		}

		rsb.result()
	}

	private def cPostFix(s: BigInt): String = if (s > (BigInt(Int.MaxValue) * 2) - 1) "ULL" else "U"

	def hardwareRegistersOutput(cpus: Seq[CpuInstance], distanceMatrix: DistanceMatrix, connected: Seq[Connected]): Unit = {
		val uniqueRegisterLists = mutable.Set[String]()

		for (cpu <- cpus) {
			Utils.ensureDirectories(hwPath(Game.outPath))

			val (_, chipAddresses) = extractRamAndRegisters(cpu, distanceMatrix, connected)

			val allRegLists = (for ((_, _, chip, _) <- chipAddresses; bank <- chip.banks) yield bank.registerListName) ++ cpu.banks.map(f => f.registerListName)

			uniqueRegisterLists ++= allRegLists.toSet
		}

		for (rl <- uniqueRegisterLists) genHeadersFor(rl)
	}

	private def extractRamAndRegisters(cpu: CpuInstance, distanceMatrix: DistanceMatrix, connected: Seq[Connected]) = {

		val ramRanges     = mutable.ArrayBuffer[(RamLike, Int, BigInt, BigInt)]()
		val chipAddresses = mutable.ArrayBuffer[(String, String, ChipInstance, BigInt)]()

		// handle cpu system registers
		if (cpu.hasInterface[RegisterBankLike]) {
			val rbi = cpu.getInterfaceUnwrapped[RegisterBankLike]
			for {rb <- rbi.banks
			     if rb.baseAddress == -1
			     if rb.cpus.isEmpty || rb.cpus.contains(cpu.cpuType)} {
				chipAddresses += ((rb.name,
					rb.registerListName.split('/').last.split('.').head.toUpperCase(),
					cpu,
					rb.baseAddress))
			}
		}

		distanceMatrix.connectedTo(cpu).filter(_.isVisibleToSoftware).foreach(ep => {
			// is possible to have several routes between chips (multiple buses) by having multiple connects with different buses
			// currently we ignore any but the first (TODO trace the correct route)
			val routeConnections: Seq[Connected] = distanceMatrix.expandedRouteBetween(cpu, ep).map {
				case (src, dst) => {
					val cons = connected.filter(con => con.connectedBetween(src, dst, FirstToSecondConnection()))
					val hd   = cons.head
					// try and swap port group for a parallel bus connection which has info about which bus was used
					if (hd.isInstanceOf[ConnectedPortGroup]) {
						val found = connected.find(p => p.isInstanceOf[ConnectedBus] &&
						                                p.firstFullName == hd.firstFullName &&
						                                p.secondFullName == hd.secondFullName)

						if (found.nonEmpty) found.get
						else cons.head
					} else cons.head
				}
			}
			var hasBus                           = false
			routeConnections.foreach {
				case _: ConnectedBus => hasBus = true
				case _               =>
			}

			if (hasBus) {
				var address: BigInt = 0
				routeConnections.foreach { r =>
					r match {
						case bus: ConnectedBus =>
							if (bus.bus.fixedBaseBusAddress)
								address = bus.bus.getBaseAddress
							else
								address += bus.bus.getBaseAddress
						case _                 =>
					}
					// only do chips once per cpu
					if (r.second.nonEmpty) {
						if (r.second.get.instance.hasInterface[RamLike]) {
							val ram   = r.second.get.instance.getInterfaceUnwrapped[RamLike]
							var index = 0
							for ((addr, size, fixed, cpus) <- ram.getRanges) {
								// per process cpu target lists
								if (cpus.isEmpty || cpus.contains(cpu.cpuType)) {
									val fa = if (fixed) addr else address + addr
									ramRanges += ((ram, index, fa, size))
									index += 1
								}
							}
						}

						val other = r.second.get.instance.asInstanceOf[ChipInstance]
						if ((!chipAddresses.exists { case (_, _, o, _) => o == other }) &&
						    other.isVisibleToSoftware &&
						    other.hasInterface[RegisterBankLike]) {
							val registerBank = other.getInterfaceUnwrapped[RegisterBankLike]
							if (other.instanceNumber >= registerBank.maxInstances)
								println(s"${other.name}: not enough instances\n")
							else for {rb <- registerBank.banks
							          if rb.cpus.isEmpty || rb.cpus.contains(cpu.cpuType)} {


								val rbName         = if (rb.name.isEmpty) r.secondFullName else rb.name
								val rbInstanceName = rbName.replace("${index}", other.instanceNumber.toString)
								chipAddresses += ((rbInstanceName,
									rb.registerListName.split('/').last.split('.').head.toUpperCase(),
									other,
									if (rb.baseAddress == -1) -1 else address + rb.baseAddress + (rb.addressIncrement * other.instanceNumber)))
							}
						}
					}
				}
			}
		})
		(ramRanges, chipAddresses)
	}

	private def genHeadersFor(registerListName: String): Unit = {
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

		sb ++= genRegisterList(s"${uname}_", rl)

		Utils.writeFile(filePath(hwPath(Game.outPath), rl.name.replace(".toml", "")), sb.result())
	}

	private def hwPath(out: Path) = hwRegsPath(out).resolve("registers")

	private def hwRegsPath(out: Path) =
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
					if (elem.description.isDefined) sb ++= f"// ${elem.description.get}%n"
					sb ++= f"#define $prefix${name}_${fieldName}_${elem.name.toUpperCase} ${elem.value.toString}%n"
				}
			})

			if (totalUserMask != 0) sb ++= f"#define $prefix${reg.name}_USERMASK ${totalUserMask & 0xFFFFFFFF}%#010xU%n"
		}

		sb.result()
	}

	private def filePath(out: Path, name: String): Path = {
		val fn       = s"${name.replace('.', '_')}.h".toLowerCase()
		val filename = Path.of(fn).getFileName
		out.resolve(filename)
	}

}

