package Overlord

private def gitAddLibrary(
    paths: Paths,
    gitUrl: String,
    optionalName: String = "",
    optionalBranch: String = ""
): Unit =
  // if no name provided, extract from gitUrl
  val name =
    if optionalName.isEmpty() then gitUrl.split('/').last.replace(".git", "")
    else optionalName
  val branch =
    if optionalBranch.isEmpty() then "main"
    else optionalBranch

  val nameAndBranch = name + "/" + branch

  // add std resource subtree
  if !os.exists(paths.targetPath / name) then
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
    assert(remoteResult.exitCode == 0)
    val mergeResult = os
      .proc("git", "merge", "-s", "ours", "--no-commit", "--allow-unrelated-histories", nameAndBranch)
      .call(
        cwd = paths.targetPath,
        check = false,
        stdout = os.Inherit,
        mergeErrIntoOut = true
      )
    assert(mergeResult.exitCode == 0)

    val subtreeResult = os
      .proc("git", "read-tree", "--prefix", name + "/", "-u", nameAndBranch)
      .call(
        cwd = paths.targetPath,
        check = false,
        stdout = os.Inherit,
        mergeErrIntoOut = true
      )
    assert(subtreeResult.exitCode == 0)
  val commitReturn = os
    .proc(
      "git",
      "commit",
      "-m",
      "Subtree merged in " + name
    )
    .call(
      cwd = paths.targetPath,
      check = false,
      stdout = os.Inherit,
      mergeErrIntoOut = true
    )
  assert(commitReturn.exitCode == 0)

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

private def gitUpdateLibrary(paths: Paths, name: String, optionalBranch: String = ""): Unit =
  val branch =
    if optionalBranch.isEmpty() then "main"
    else optionalBranch

  val updateGit = os
    .proc(
      "git",
      "pull",
      "-s",
      "subtree",
      name,
      "main"
    )
    .call(
      cwd = paths.targetPath,
      check = false,
      stdout = os.Inherit,
      mergeErrIntoOut = true
    )
  assert(updateGit.exitCode == 0)
