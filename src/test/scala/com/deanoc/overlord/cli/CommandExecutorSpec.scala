package com.deanoc.overlord.cli

import org.scalatest.flatspec.AnyFlatSpec
import org.scalatest.matchers.should.Matchers

class CommandExecutorSpec extends AnyFlatSpec with Matchers {

  "CommandExecutor" should "handle unknown commands gracefully" in {
    val config = Config(command = Some("unknown"), subCommand = Some("command"))
    val result = CommandExecutor.execute(config)
    result should be(false)
  }

  it should "handle missing subcommands gracefully" in {
    val config = Config(command = Some("create"))
    val result = CommandExecutor.execute(config)
    result should be(false)
  }

  it should "handle missing required parameters gracefully" in {
    val config = Config(command = Some("create"), subCommand = Some("project"))
    val result = CommandExecutor.execute(config)
    result should be(false)
  }
}
