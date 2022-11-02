package Overlord

private def gitInit(paths: Paths, optionalBranch: String = ""): Unit =
  val branch =
    if optionalBranch.isEmpty() then "main"
    else optionalBranch

  // initialise git
  val initReturn = os
    .proc(
      "git",
      "init"
// need newer git      "--initial-branch=main"
    )
    .call(
      cwd = paths.targetPath,
      check = false,
      stdout = os.Inherit,
      mergeErrIntoOut = true
    )
  assert(initReturn.exitCode == 0)
  val oldSkoolInitialBranchReturn = os
    .proc(
      "git",
      "checkout",
      "-b",
      branch
    )
    .call(
      cwd = paths.targetPath,
      check = false,
      stdout = os.Inherit,
      mergeErrIntoOut = true
    )
  assert(initReturn.exitCode == 0)

  os.write(paths.targetPath / ".gitignore", "")
  val addReturn = os
    .proc(
      "git",
      "add",
      ".gitignore"
    )
    .call(
      cwd = paths.targetPath,
      check = false,
      stdout = os.Inherit,
      mergeErrIntoOut = true
    )
  assert(addReturn.exitCode == 0)
private def gitCommit(paths: Paths, msg: String): Unit =
  val commitReturn = os
    .proc(
      "git",
      "commit",
      "-m",
      msg
    )
    .call(
      cwd = paths.targetPath,
      check = false,
      stdout = os.Inherit,
      mergeErrIntoOut = true
    )
  assert(commitReturn.exitCode == 0)

def gitClone(paths: Paths, name: String, optionalDest: String = ""): Unit =
  if optionalDest.isEmpty then
    val cloneGit = os
      .proc(
        "git",
        "clone",
        name
      )
      .call(
        cwd = paths.targetPath,
        check = false,
        stdout = os.Inherit,
        mergeErrIntoOut = true
      )
    assert(cloneGit.exitCode == 0)
  else
    val cloneGit = os
      .proc(
        "git",
        "clone",
        name,
        optionalDest
      )
      .call(
        cwd = paths.targetPath,
        check = false,
        stdout = os.Inherit,
        mergeErrIntoOut = true
      )
    assert(cloneGit.exitCode == 0)

private def gitAddLibSubTree(
    paths: Paths,
    gitUrl: String,
    optionalName: String = "",
    optionalBranch: String = "",
    optionalPrefix: String = "libs"
): Unit =
  // if no name provided, extract from gitUrl
  val name =
    if optionalName.isEmpty() then gitUrl.split('/').last.replace(".git", "")
    else optionalName
  val branch =
    if optionalBranch.isEmpty() then "main"
    else optionalBranch

  if !os.exists(paths.libPath / name) then
    val remoteResult = os
      .proc(
        "git",
        "remote",
        "add",
        "-f",
        name,
        gitUrl
      )
      .call(
        cwd = paths.targetPath,
        check = false,
        stdout = os.Inherit,
        mergeErrIntoOut = true
      )
    // assert(remoteResult.exitCode == 0)
    val mergeResult = os
      .proc(
        "git",
        "subtree",
        "add",
        "--prefix",
        s"$optionalPrefix/$name",
        name,
        branch
      )
      .call(
        cwd = paths.targetPath,
        check = false,
        stdout = os.Inherit,
        mergeErrIntoOut = true
      )
    assert(mergeResult.exitCode == 0)

private def gitPushLibSubTree(
    paths: Paths,
    gitUrl: String,
    optionalName: String = "",
    optionalBranch: String = "",
    optionalPrefix: String = "libs"
): Unit =
  // if no name provided, extract from gitUrl
  val name =
    if optionalName.isEmpty() then gitUrl.split('/').last.replace(".git", "")
    else optionalName
  val branch =
    if optionalBranch.isEmpty() then "main"
    else optionalBranch

  val mergeResult = os
    .proc(
      "git",
      "subtree",
      "push",
      "--prefix",
      s"$optionalPrefix/$name",
      name,
      branch
    )
    .call(
      cwd = paths.targetPath,
      check = false,
      stdout = os.Inherit,
      mergeErrIntoOut = true
    )
  // allow errors as we might not have rights to push some libraries
  // assert(mergeResult.exitCode == 0)

private def gitUpdateLibrary(
    paths: Paths,
    gitUrl: String,
    optionalName: String = "",
    optionalBranch: String = "",
    optionalPrefix: String = "libs"
): Unit =
  // if no name provided, extract from gitUrl
  val name =
    if optionalName.isEmpty() then gitUrl.split('/').last.replace(".git", "")
    else optionalName

  val branch =
    if optionalBranch.isEmpty() then "main"
    else optionalBranch

  val updateGit = os
    .proc(
      "git",
      "subtree",
      "pull",
      "--prefix",
      s"$optionalPrefix/$name",
      name,
      branch
    )
    .call(
      cwd = paths.targetPath,
      check = false,
      stdout = os.Inherit,
      mergeErrIntoOut = true
    )
  assert(updateGit.exitCode == 0)
