package output

import overlord.Gateware.BitsDesc

import java.io.{BufferedWriter, FileWriter}
import java.nio.file.Path
import overlord.Instances.{CpuInstance, Instance}
import overlord.Software.Register
import overlord._

import scala.collection.mutable
import scala.xml.PrettyPrinter

object Svd {
	//@formatter:off
	private def outputFakeCpu(cpu: CpuInstance) ={
		<name>{cpu.ident.split('.')(0) match {
			case "a53" => "CA53"
			case "a9" =>"CA9"
			case "a57" => "CA57"
			case "a72" => "CA72"
			case _ => "other"}}</name>
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
	private def outputCpu(cpu: CpuInstance) = {
		<name>{cpu.ident}</name>
		<endian>little</endian>
	}

	private def outputRegister(r: Register) =
	<register>
		<name>{r.name}</name>
		<description>{r.desc}</description>
		<addressOffset>{f"${r.offset}"}</addressOffset>
		<size>{r.width}</size>
		<resetValue>{f"${r.default}"}</resetValue>
		if(r.fields.length > 0)
		<fields> {
			for (f <- r.fields) yield {
				val bits = BitsDesc(f.bits)
				<field>
					<name>{f.name}</name>
					<description> {
						if (f.longDesc.nonEmpty) f.longDesc.get
						else if(f.shortDesc.nonEmpty) f.shortDesc }
					</description>
					<lsb>{bits.lo}</lsb>
					<msb>{bits.hi}</msb>
				</field>
			}
			}
		</fields>
	</register>

	private val definitionsWritten = mutable.HashMap[String, String]()

	private def outputPeripheral(s: Instance) : Seq[xml.Elem] ={
		if(s.definition.software.isEmpty) Seq[xml.Elem]()
		else for(regs <- s.definition.software.get.groups) yield {
				val bank = if(s.attributes.contains("bank")) {
					regs.banks.find(_.name == s.attributes("bank")
						                .asInstanceOf[toml.Value.Str].value) match {
							case Some(value) => value
							case None =>
								println(s"Peripheral ${s.ident} bank name is not found in " +
								        s"the definition")
								return Array[xml.Elem]()
						}
					} else if(definitionsWritten.contains(s.definition.defType.toString)) {
						println(s"Peripheral ${s.ident} contains no bank name but is " +
						        s"used in a multiple instance definition")
						return Array[xml.Elem]()
					} else regs.banks.head

				if(definitionsWritten.contains(s.definition.defType.toString)) {
					// derivedFrom case
					val derivedFromName = definitionsWritten(s.definition.defType.toString)
					<peripheral derivedFrom={derivedFromName}>
					<name>{bank.name}</name>
					<version>1.0</version>
					<description>{regs.description}</description>
					<baseAddress>{f"${bank.address}"}</baseAddress>
				</peripheral>
			} else {
				// new definition
				definitionsWritten += (s.definition.defType.toString -> bank.name)
				<peripheral>
					<name>{bank.name}</name>
					<version>1.0</version>
					<description>{regs.description}</description>
					<baseAddress>{f"${bank.address}"}</baseAddress>
					<addressBlock>
						<offset>0x0</offset>
						<size>{regs.bankSize}</size>
						<usage>registers</usage>
						<protection>s</protection>
					</addressBlock>
					<registers>
						{ for (r <- regs.registers) yield outputRegister(r) }
					</registers>
				</peripheral>
			}
		}
	}

	def apply(game: Game, out : Path): Unit = {
		val cpus = for (cpu <- game.cpus) yield <cpu>{outputCpu(cpu)}</cpu>

		val peripherals = for (p <- game.peripherals) yield outputPeripheral(p)

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
				<peripherals>{peripherals}</peripherals>
				<vendorExtensions>
					<cpus>{cpus}</cpus>
				</vendorExtensions>
			</device>
	//@formatter:on

		ensureDirectories(out)
		val path =
			if (out.toFile.isDirectory) out.resolve(s"${
				game.name
			}.svd")
			else out

		writeFile(path, new PrettyPrinter(Int.MaxValue, 2).format(deviceXml))
	}

	private def ensureDirectories(path: Path): Unit = {
		val directory = path.toFile
		if (directory.isDirectory && !directory.exists()) {
			directory.mkdirs()
		}
	}

	private def writeFile(path: Path, s: String): Unit = {
		val file = path.toFile
		val bw   = new BufferedWriter(new FileWriter(file))
		bw.write(s)
		bw.close()
	}



	/* TODO
		<dataType>{f"uint${1 << log2Up(r.defi.width.max(8))}_t"}</dataType>
	*/

}
