package com.deanoc.overlord.hardware

import com.deanoc.overlord.utils.{StringV, Utils, Variant}
import com.deanoc.overlord.Overlord
import com.deanoc.overlord.instances.InstanceTrait

import java.nio.file.Path
import scala.collection.mutable
import scala.util.boundary, boundary.break

case class RegisterBank(
    name: String,
    baseAddress: BigInt,
    addressIncrement: BigInt,
    registerWindowSize: BigInt,
    registerListName: String,
    cpus: Seq[String]
)

case class Register(
    name: String,
    regType: String,
    width: Int,
    default: BigInt,
    offset: BigInt,
    desc: String,
    fields: Array[RegisterField]
)

case class RegisterList(
    name: String,
    description: String,
    registers: Array[Register]
) {
  def sizeInBytes: BigInt = registers.last.offset

}

case class RegisterField(
    name: String,
    bits: String,
    accessType: String,
    enums: Array[RegisterFieldEnum],
    shortDesc: Option[String],
    longDesc: Option[String]
)

case class RegisterFieldEnum(
    name: String,
    value: BigInt,
    description: Option[String]
)

object Registers {
  val registerListCache: mutable.Map[String, RegisterList] =
    mutable.Map[String, RegisterList]()

  private val cpuRegEx = "\\s*,\\s*".r

  private def decodeCpusString(cpus: String): Seq[String] =
    if (cpus == "_") Seq() else cpuRegEx.split(cpus).toSeq.map(_.toLowerCase())

  def apply(
      instance: InstanceTrait,
      registerDefs: Seq[Variant]
  ): Seq[RegisterBank] = {

    val yamls = mutable.ArrayBuffer[(String, Map[String, Variant])]()

    val registerBanks = for (inlineTable <- registerDefs) yield {
      val item = Utils.toTable(inlineTable)
      boundary {
        if (!item.contains("resource")) {
          println(s"No resource in register table\n")
          break(Seq())
        }
        val resource = Utils.toString(item("resource"))
        val name = Utils.lookupString(item, "name", "")
        val baseAddress = Utils.lookupBigInt(item, "base_address", -1)
        val addressIncrement = Utils.lookupBigInt(item, "address_increment", 0)
        val registerWindowSize =
          Utils.lookupBigInt(item, "register_window_size", -1)
        val cpus = decodeCpusString(Utils.lookupString(item, "cpus", "_"))

        val path: Path = Overlord.tryPaths(instance, resource)

        if (path == null) {
          println(s"RegisterBank $name resource $resource path $path not found")
          break(Seq())
        }

        break(Seq())
/* 

        val source = Utils.readYaml(path)
        yamls += ((resource, source))

        if (Registers.registerListCache.contains(resource)) {
          Some(
            RegisterBank(
              name,
              baseAddress,
              addressIncrement,
              registerWindowSize,
              resource,
              cpus
            )
          )
        } else
          parseRegisterList(resource, source).flatMap { list =>
            Registers.registerListCache(resource) = list
            Some(
              RegisterBank(
                name,
                baseAddress,
                addressIncrement,
                registerWindowSize,
                resource,
                cpus
              )
            )
          }
          */
        }
    }
    registerBanks.flatten
  }

  private def parseRegisterList(
      name: String,
      parsed: Map[String, Variant]
  ): Option[RegisterList] = {
    if (!parsed.contains("register")) return None

    val tregisters = Utils.toArray(parsed("register"))
    val registers = for (reg <- tregisters) yield {
      val table = Utils.toTable(reg)
      val regName = Utils.toString(table("name"))
      val regType =
        if (table.contains("type")) Utils.toString(table("type"))
        else {
          "rw"
        }
      val width = Utils.toInt(table("width"))
      val default =
        if (table.contains("type")) Utils.toBigInt(table("default"))
        else {
          BigInt(0)
        }
      val offset = Utils.toBigInt(table("offset"))
      val desc = if (table.contains("description")) {
        Utils.toString(table("description"))
      } else {
        ""
      }

      val fields = if (table.contains("field")) {
        for (field <- Utils.toArray(table("field"))) yield {
          val table = Utils.toTable(field)
          val fieldName = Utils.toString(table("name"))
          val fieldBits = Utils.toString(table("bits"))
          val fieldType = if (table.contains("type")) {
            Utils.toString(table("type"))
          } else {
            ""
          }

          val shortDesc =
            if (table.contains("shortdesc"))
              Some(Utils.toString(table("shortdesc")))
            else if (table.contains("description"))
              Some(Utils.toString(table("description")))
            else None

          val longDesc =
            if (table.contains("longdesc"))
              Some(Utils.toString(table("longdesc")))
            else None

          val enums = if (table.contains("enum")) {
            for (reg_enum <- Utils.toArray(table("enum"))) yield {
              val table = Utils.toTable(reg_enum)
              val enumName = Utils.toString(table("name"))
              val enumValue = Utils.toBigInt(table("value"))
              val enumDesc =
                if (table.contains("description"))
                  Some(Utils.toString(table("description")))
                else None
              RegisterFieldEnum(enumName, enumValue, enumDesc)
            }
          } else Array[RegisterFieldEnum]()

          RegisterField(
            fieldName,
            fieldBits,
            fieldType,
            enums,
            shortDesc,
            longDesc
          )
        }
      } else Array[RegisterField]()

      Register(regName, regType, width, default, offset, desc, fields)
    }
    val desc =
      if (parsed.contains("description"))
        parsed("description").asInstanceOf[StringV].value
      else "No Description"
    val finalName =
      if (parsed.contains("name"))
        parsed("name").asInstanceOf[StringV].value
      else name

    Some(RegisterList(finalName, desc, registers))
  }
}
