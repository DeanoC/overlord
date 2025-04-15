package com.deanoc.overlord

import com.deanoc.overlord.utils._
import com.deanoc.overlord.hardware.{BitsDesc, RegisterList, Registers}
import com.deanoc.overlord.connections._
import com.deanoc.overlord.connections.ConnectionDirection
import com.deanoc.overlord.instances._
import com.deanoc.overlord.interfaces.{RamLike, RegisterBankLike}
import com.deanoc.overlord.Overlord
import com.deanoc.overlord.connections.ConnectedExtensions._

import java.nio.file.{Path, Paths}
import scala.collection.mutable

object OutputSoftware {
  def cpuInvariantActions(
      software: Seq[SoftwareInstance],
      constants: Seq[Constant]
  ): Unit = {
    for (instance <- software)
      executeCpuInvariantSoftwareActions(instance, constants)
  }

  /** Executes CPU-invariant software actions for a given software instance.
    * These actions are filtered by phase 1 and executed with merged parameters.
    */
  private def executeCpuInvariantSoftwareActions(
      instance: SoftwareInstance,
      constants: Seq[Constant]
  ): Unit = {
    val software = instance.definition
    val actions = software.actionsFile.actions

    for (action <- actions.filter(_.phase == 1)) {
      val conParameters = constants
        .collect { case cc: Constant => cc }
        .map(c => {
          val name =
            if (c.parameter.name.isEmpty) c.instance.name else c.parameter.name
          c.parameter.parameterType match {
            case ConstantParameterType(value) =>
              Map[String, Variant](name -> value)
            case FrequencyParameterType(freq) =>
              Map[String, Variant](name -> DoubleV(freq))
          }
        })
        .fold(Map[String, Variant]())((o, n) => o ++ n)

      val instanceSpecificParameters = instance match {
        case _ => Map[String, Variant]()
      }

      val parameters =
        software.parameters ++ conParameters ++ instanceSpecificParameters

      instance.mergeAllAttributes(parameters)

      instance.finalParameterTable.addAll(
        for ((k, v) <- parameters) yield k -> v
      )

      action.execute(instance, instance.finalParameterTable.toMap)
    }

  }

  def cpuSpecificActions(
      boardName: String,
      software: Seq[SoftwareInstance],
      cpus: Seq[CpuInstance],
      distanceMatrix: DistanceMatrix,
      connected: Seq[Connected]
  ): Unit = {
    val keywordsPerCpu: Array[Map[String, Variant]] = (for (cpu <- cpus) yield {
      val sanitizedTriple = cpu.triple.replace("-", "_")
      val memMap = generateMemoryMapFor(cpu, distanceMatrix, connected)
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

    for (instance <- software)
      executeCpuSpecificSoftwareActions(instance, cpus, keywordsPerCpu)
  }

  /** Executes CPU-specific software actions for a given software instance.
    * These actions are filtered by phase 2 and executed with CPU-specific
    * parameters.
    */
  private def executeCpuSpecificSoftwareActions(
      instance: SoftwareInstance,
      cpus: Seq[CpuInstance],
      keywordsPerCpu: Array[Map[String, Variant]]
  ): Unit = {
    val actions = instance.definition.actionsFile.actions

    for (action <- actions.filter(_.phase == 2)) {
      for (iCpu <- cpus.indices)
        action.execute(
          instance,
          keywordsPerCpu(iCpu) + ("${name}" -> StringV(
            instance.name
          )) ++ instance.finalParameterTable.toMap
        )
    }
  }

  /** Generates a memory map for a specific CPU based on its connections and
    * distance matrix. The memory map includes RAM ranges and chip addresses.
    */
  private def generateMemoryMapFor(
      cpu: CpuInstance,
      distanceMatrix: DistanceMatrix,
      connected: Seq[Connected]
  ): String = {
    val mmsb = new mutable.StringBuilder

    val mm = genChipMemoryMapFor(cpu, distanceMatrix, connected)
    mmsb ++= mm

    mmsb.result()
  }

  private def genChipMemoryMapFor(
      cpu: CpuInstance,
      distanceMatrix: DistanceMatrix,
      connected: Seq[Connected]
  ): String = {
    // Extract RAM ranges and chip addresses for the given CPU
    val (ramRanges, chipAddresses) =
      extractRamAndRegisters(cpu, distanceMatrix, connected)

    // Add CPU-specific register banks to chip addresses
    addCpuRegisterBanks(cpu, chipAddresses)

    // Generate memory map strings for RAM ranges and chip addresses
    val ramMap = generateRamMap(ramRanges.toSeq)
    val chipMap = generateChipMap(chipAddresses.toSeq)

    // Combine and return the memory map
    ramMap + chipMap
  }

  /** Adds CPU-specific register banks to the chip addresses list.
    */
  private def addCpuRegisterBanks(
      cpu: CpuInstance,
      chipAddresses: mutable.ArrayBuffer[(String, String, ChipInstance, BigInt)]
  ): Unit = {
    for (rb <- cpu.banks if rb.baseAddress != -1) {
      chipAddresses += ((rb.name, rb.name, cpu, rb.baseAddress))
    }
  }

  /** Generates memory map definitions for RAM ranges.
    */
  private def generateRamMap(
      ramRanges: Seq[(RamLike, Int, BigInt, BigInt)]
  ): String = {
    val ramSizesByRam = mutable.Map[String, BigInt]()
    val rsb = new mutable.StringBuilder

    // Aggregate RAM sizes by owner
    for ((ram, _, _, size) <- ramRanges) {
      val si = ram.getOwner.name.replace('.', '_')
      ramSizesByRam.updateWith(si) {
        case Some(existingSize) => Some(existingSize + size)
        case None               => Some(size)
      }
    }

    // Generate RAM size definitions
    for ((name, size) <- ramSizesByRam) {
      rsb ++= f"%n#define ${name.toUpperCase}_TOTAL_SIZE_IN_BYTES $size${cPostFix(size)}%n"
    }

    // Generate RAM base address and size definitions
    for ((ram, i, address, size) <- ramRanges) {
      rsb ++= f"%n"
      val si = ram.getOwner.name.replace('.', '_')
      val name = s"${si}_$i".toUpperCase
      rsb ++= f"#define ${name}_BASE_ADDR 0x$address%x${cPostFix(address)}%n"
      rsb ++= f"#define ${name}_SIZE_IN_BYTES $size${cPostFix(size)}%n"
    }

    rsb.result()
  }

  /** Generates memory map definitions for chip addresses.
    */
  private def generateChipMap(
      chipAddresses: Seq[(String, String, ChipInstance, BigInt)]
  ): String = {
    val rsb = new mutable.StringBuilder

    for ((rbName, rlName, _, address) <- chipAddresses) {
      val rbNameU = rbName.toUpperCase
      val rlNameU = rlName.toUpperCase

      if (address >= 0) {
        rsb ++= f"%n#define ${rbNameU}_BASE_ADDR 0x$address%x${cPostFix(address)}%n"
      } else {
        rsb ++= f"// These registers aren't MMIO accessible so require platform intrinsics to use%n"
      }
    }

    rsb.result()
  }

  private def cPostFix(s: BigInt): String =
    if (s > (BigInt(Int.MaxValue) * 2) - 1) "ULL" else "U"

  def hardwareRegistersOutput(
      cpus: Seq[CpuInstance],
      distanceMatrix: DistanceMatrix,
      connected: Seq[Connected]
  ): Unit = {
    val uniqueRegisterLists = mutable.Set[String]()

    for (cpu <- cpus) {
      val outPath = Overlord.outPath
      Utils.ensureDirectories(hwPath(outPath))

      val (_, chipAddresses) =
        extractRamAndRegisters(cpu, distanceMatrix, connected)

      val allRegLists =
        (for ((_, _, chip, _) <- chipAddresses; bank <- chip.banks)
          yield bank.registerListName) ++ cpu.banks.map(f => f.registerListName)

      uniqueRegisterLists ++= allRegLists.toSet
    }

    for (rl <- uniqueRegisterLists) genHeadersFor(rl)
  }

  /** Extracts RAM ranges and register addresses for a given CPU based on its
    * connections. This includes handling RAM-like interfaces and register
    * banks.
    */
  private def extractRamAndRegisters(
      cpu: CpuInstance,
      distanceMatrix: DistanceMatrix,
      connected: Seq[Connected]
  ) = {

    val ramRanges = mutable.ArrayBuffer[(RamLike, Int, BigInt, BigInt)]()
    val chipAddresses =
      mutable.ArrayBuffer[(String, String, ChipInstance, BigInt)]()

    // handle cpu system registers
    for {
      rbi <- cpu.getInterface[RegisterBankLike]
      rb <- rbi.banks
      if rb.baseAddress == -1
      if rb.cpus.isEmpty || rb.cpus.contains(cpu.cpuType)
    } {
      chipAddresses += ((
        rb.name,
        rb.registerListName.split('/').last.split('.').head.toUpperCase(),
        cpu,
        rb.baseAddress
      ))
    }

    distanceMatrix
      .connectedTo(cpu)
      .filter(_.isVisibleToSoftware)
      .foreach(ep => {
        // is possible to have several routes between chips (multiple buses) by having multiple connects with different buses
        // currently we ignore any but the first (TODO: trace the correct route)
        val routeConnections: Seq[Connected] =
          distanceMatrix.expandedRouteBetween(cpu, ep).map {
            case (src, dst) => {
              val cons = connected.filter(con =>
                con
                  .connectedBetween(src, dst, ConnectionDirection.FirstToSecond)
              )
              val hd = cons.head
              // try and swap port group for a parallel bus connection which has info about which bus was used
              if (hd.isInstanceOf[ConnectedPortGroup]) {
                val found = connected.find(p =>
                  p.isInstanceOf[ConnectedBus] &&
                    p.firstFullName == hd.firstFullName &&
                    p.secondFullName == hd.secondFullName
                )

                if (found.nonEmpty) found.get
                else cons.head
              } else cons.head
            }
          }
        var hasBus = false
        routeConnections.foreach {
          case bus: ConnectedBus => hasBus = true
          case _                 =>
        }

        if (hasBus) {
          var address: BigInt = 0
          routeConnections.foreach { r =>
            r match {
              case bus: ConnectedBus =>
                if (bus.bus.fixedBaseBusAddress)
                  address = bus.bus.getBaseAddress
                else address += bus.bus.getBaseAddress
              case _ =>
            }
            // only do chips once per cpu
            if (r.second.nonEmpty) {
              r.second.get.instance.getInterface[RamLike].foreach { ram =>
                var index = 0
                for ((addr, size, fixed, cpus) <- ram.getRanges) {
                  // per process cpu target lists
                  if (cpus.isEmpty || cpus.contains(cpu.cpuType)) {
                    val fa = if (fixed) addr else address + addr
                    ramRanges += ((ram, index, fa, size))
                    index += 1
                  }
                }
                true
              }

              val other = r.second.get.instance.asInstanceOf[ChipInstance]
              handleConnectedChip(cpu, other, address, chipAddresses)
            }
          }
        }
      })
    (ramRanges, chipAddresses)
  }

  private def handleConnectedChip(
      cpu: CpuInstance,
      other: ChipInstance,
      address: BigInt,
      chipAddresses: mutable.ArrayBuffer[(String, String, ChipInstance, BigInt)]
  ): Unit = {
    if (
      !chipAddresses.exists { case (_, _, o, _) => o == other } &&
      other.isVisibleToSoftware &&
      other.hasInterface[RegisterBankLike]
    ) {
      other.getInterface[RegisterBankLike].foreach { registerBank =>
        processRegisterBank(cpu, other, registerBank, address, chipAddresses)
      }
    }
  }

  private def processRegisterBank(
      cpu: CpuInstance,
      other: ChipInstance,
      registerBank: RegisterBankLike,
      address: BigInt,
      chipAddresses: mutable.ArrayBuffer[(String, String, ChipInstance, BigInt)]
  ): Boolean = {
    if (other.instanceNumber >= registerBank.maxInstances) {
      println(s"${other.name}: not enough instances\n")
      false
    } else {
      registerBank.banks
        .filter(rb => rb.cpus.isEmpty || rb.cpus.contains(cpu.cpuType))
        .foreach { rb =>
          val rbName = if (rb.name.isEmpty) other.name else rb.name
          val rbInstanceName =
            rbName.replace("${index}", other.instanceNumber.toString)
          val calculatedAddress: BigInt =
            if (rb.baseAddress == -1) BigInt(-1)
            else
              address + rb.baseAddress + (rb.addressIncrement * other.instanceNumber)
          chipAddresses += ((
            rbInstanceName,
            rb.registerListName.split('/').last.split('.').head.toUpperCase(),
            other,
            calculatedAddress
          ))
        }
      true
    }
  }

  /** Generates header files for a given register list. The headers define
    * register offsets, masks, and other metadata.
    */
  private def genHeadersFor(registerListName: String): Unit = {
    // output register definitions
    val rl = Registers.registerListCache(registerListName)
    val name = rl.name.split('/').last.replace(".yaml", "")
    val uname = name.toUpperCase()

    val sb = new mutable.StringBuilder
    sb ++= f"#pragma once%n"
    sb ++= f"// Copyright Deano Calver%n"
    sb ++= f"// SPDX-License-Identifier: MIT%n"
    sb ++= f"// ${rl.name}%n"
    if (rl.description.nonEmpty) sb ++= f"// ${rl.description}%n"

    sb ++= genRegisterList(s"${uname}_", rl)

    Utils.writeFile(
      filePath(hwPath(Overlord.outPath), rl.name.replace(".yaml", "")),
      sb.result()
    )
  }

  private def hwPath(out: Path) = hwRegsPath(out).resolve("registers")

  private def hwRegsPath(out: Path) =
    out
      .resolve("libs")
      .resolve("platform")
      .resolve("include")
      .resolve("platform")

  private def genRegisterList(prefix: String, rl: RegisterList): String = {
    val sb = new mutable.StringBuilder
    for (reg <- rl.registers) {
      sb ++= f"%n// ${reg.desc}%n"
      sb ++= f"#define $prefix${reg.name}_OFFSET ${reg.offset}%#010xU%n"

      var totalUserMask: Long = 0
      var reservedFieldCount: Long =
        -1 // we preincrement so start at -1 to get 0

      reg.fields.foreach(f => {
        sb ++= f"//${if (f.shortDesc.nonEmpty) f.shortDesc.get else ""}%n"

        val bits = BitsDesc(f.bits)
        totalUserMask |= (if (f.name != "RESERVED") bits.mask else 0)

        val fieldName = if (f.name == "RESERVED") {
          reservedFieldCount += 1
          s"RESERVED$reservedFieldCount"
        } else f.name

        val name = reg.name.toUpperCase().replace('.', '_')

        sb ++= f"#define $prefix${name}_${fieldName}_LSHIFT ${bits.lo}%#010xU%n"
        sb ++= f"#define $prefix${name}_${fieldName}_MASK ${bits.mask}%#010xU%n"
        if (bits.singleBit)
          sb ++= f"#define $prefix${name}_$fieldName ${bits.mask}%#10xU%n"

        for (elem <- f.enums) {
          if (elem.description.isDefined) sb ++= f"// ${elem.description.get}%n"
          sb ++= f"#define $prefix${name}_${fieldName}_${elem.name.toUpperCase} ${elem.value.toString}%n"
        }
      })

      if (totalUserMask != 0)
        sb ++= f"#define $prefix${reg.name}_USERMASK ${totalUserMask & 0xffffffff}%#010xU%n"
    }

    sb.result()
  }

  /** Constructs the file path for a generated header file based on the output
    * directory and name.
    */
  private def filePath(out: Path, name: String): Path = {
    val fn = s"${name.replace('.', '_')}.h".toLowerCase()
    val filename = Paths.get(fn).getFileName
    out.resolve(filename)
  }

}
