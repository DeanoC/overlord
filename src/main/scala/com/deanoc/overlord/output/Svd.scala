package com.deanoc.overlord.output

import com.deanoc.overlord.Hardware.{BitsDesc, Register}
import com.deanoc.overlord.utils.Utils
import com.deanoc.overlord.Instances.{ChipInstance, CpuInstance}
import com.deanoc.overlord._
import com.deanoc.overlord.Project

import java.nio.file.{Path, Paths}
import scala.collection.mutable
import scala.xml.PrettyPrinter

object Svd {
  def apply(game: Project): Unit = {

    val cpus =
      for (cpu <- game.cpus)
        yield <cpu>
        {outputCpu(cpu)}
      </cpu>

    val instances = for (p <- game.allChipInstances) yield outputRegisters(p)

    val deviceXml: xml.Elem =
      <device schemaVersion="1.3"
              xmlns:xs="http://www.w3.org/2001/XMLSchema-instance"
              xs:noNamespaceSchemaLocation="etc/CMSIS-SVD.xsd">
        <vendor>overlord</vendor>
        <vendorID>OVER</vendorID>
        <name>{game.name}</name>
        <version>0.0.0</version>
        <description>{game.name}</description>
        <licenseText>// SPDX-License-Identifier: MIT\n</licenseText>
        <cpu>{outputFakeCpu(game.cpus.head)}</cpu>
        <addressUnitBits>8</addressUnitBits>
        <width>32</width>
        <size>32</size>
        <access>read-write</access>
        <resetValue>0x00000000</resetValue>
        <resetMask>0xFFFFFFFF</resetMask>
        <peripherals>{instances}</peripherals>
        <vendorExtensions>
          <cpus>{cpus}</cpus>
        </vendorExtensions>
      </device>
    // @formatter:on

    val out = Project.outPath
    Utils.ensureDirectories(out)

    // copy etc/CMSIS-SVD.xsd
    val etc = out.resolve("etc")
    Utils.ensureDirectories(etc)
    Utils.copy(Paths.get("etc/CMSIS-SVD.xsd"), etc.resolve("CMSIS-SVD.xsd"))

    val path =
      if (out.toFile.isDirectory)
        out.resolve(s"${game.name}.svd")
      else out

    Utils.writeFile(path, new PrettyPrinter(Int.MaxValue, 2).format(deviceXml))
  }

  //@formatter:off
  private def outputFakeCpu(cpu: CpuInstance) = {
    <name>{cpu.name.split('.')(0) match {
      case "a53" => "CA53"
      case "a9" => "CA9"
      case "a57" => "CA57"
      case "a72" => "CA72"
      case _ => "other"
    }}</name>
    <revision>r0p0</revision>
    <endian>little</endian>
    <mpuPresent>false</mpuPresent>
    <fpuPresent>true</fpuPresent>
    <icachePresent>true</icachePresent>
    <dcachePresent>true</dcachePresent>
    <itcmPresent>false</itcmPresent>
    <dtcmPresent>false</dtcmPresent>
    <vtorPresent>false</vtorPresent>
    <nvicPrioBits>0</nvicPrioBits>
    <vendorSystickConfig>false</vendorSystickConfig>
  }

  private def outputRegister(r: Register) = {
    <register>
      <name>{r.name}</name>
      <description>{
        if (r.desc.nonEmpty) r.desc
        else "No Description"
      }</description>
      <addressOffset>{f"${r.offset}"}</addressOffset>
      <size>{r.width}</size>
      <resetValue>{f"${r.default}"}</resetValue>
      {if (r.fields.nonEmpty)
        <fields>{
          for (f <- r.fields) yield {
            val bits = BitsDesc(f.bits)
            <field>
              <name>{f.name}</name>
              <description>{
                if (f.longDesc.nonEmpty) f.longDesc.get
                else if (f.shortDesc.nonEmpty) f.shortDesc.get
                else "No Description"
              }</description>
              <lsb>{bits.lo}</lsb>
              <msb>{bits.hi}</msb>
            </field>
          }
        }</fields>
      }
    </register>
  }

  private val definitionsWritten = mutable.HashMap[String, String]()

  private def outputCpu(cpu: CpuInstance) = {
    <name>{cpu.name}</name>
    <endian>little</endian>
  }

  private def outputRegisters(s: ChipInstance): Seq[xml.Elem] = ???
}
