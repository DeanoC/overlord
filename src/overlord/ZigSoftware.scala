package Overlord

import scala.collection.mutable

object ZigSoftware:
  def programsTop(paths: Paths, zigLocalPrograms: Seq[SoftwareDef], libsById: Map[Identifier, SoftwareDef]): String =
    val sb = mutable.StringBuilder()
    sb ++= """
const std = @import("std");
const Library = @import("libs/library.zig").Library;
const sdkPath = @import("libs/library.zig").sdkPath;
"""
    sb ++= "\n"

    // insert local programs import
    zigLocalPrograms.foreach(sw =>
      sw match
        case ProgramSoftwareDef(name, boards, cpus, builder, libraries, dependencies, actions) =>
          sb ++= s"""
const $name = @import("$name/ikuy_build.zig");
"""
        case _ => {}
    )
    sb ++= "\n"
    // insert all zig link function first
    libsById.foreach((id, sw) =>
      sw match
        case LibSoftwareDef(name, boards, cpus, builder, libraries, dependencies, actions, zig) =>
          zig match
            case None =>
            case Some(z) =>
              val dictionary = Map("${lib_name}" -> name)
              sb ++= z.link.replace("\\n", "\n").overlordStringInterpolate(dictionary)
              sb ++= "\n"
        case _ => {}
    )
    sb ++= "\n"

    // add zig build header and place to store libraries
    sb ++= """
pub fn build(b: *std.build.Builder) !void {
  var libraryPackages = std.StringHashMap(Library).init(b.allocator);
"""
    // add libraries for each one
    libsById.foreach((id, sw) =>
      sw match
        case LibSoftwareDef(name, boards, cpus, builder, libraries, dependencies, actions, zig) =>
          val dictionary = Map("${lib_name}" -> name)
          val str =
            if zig.nonEmpty then
              s"""  try libraryPackages.put("$name", Library{ .link = &sdl2Link, .name = "$name", .path = sdkPath("/$name/build.zig") });""" + "\n"
            else
              s"""  try libraryPackages.put("$name", Library{ .name = "$name", .path = sdkPath("/$name/build.zig") });""" + "\n"

          sb ++= str.overlordStringInterpolate(dictionary)

        case _ => {}
    )
    // insert local programs build call
    zigLocalPrograms.foreach(sw =>
      sw match
        case ProgramSoftwareDef(name, boards, cpus, builder, libraries, dependencies, actions) =>
          sb ++= s"""
  $name.build(b, libraryPackages);
"""
        case _ => {}
    )

    // close the build functions
    sb ++= "}\n"

    sb.result
