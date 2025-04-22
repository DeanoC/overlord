package com.deanoc.overlord

enum ProjectAccess {
  case ReadOnly, Update, Generate
}
object GlobalState {
  // default to not allowing writes to the project (logs/info are always allowed)
  private var projectAccess: ProjectAccess = ProjectAccess.ReadOnly
  
  def setProjectToGenerateMode(): Unit = {
    projectAccess = ProjectAccess.Generate
  }

  def setProjectToUpdateMode(): Unit = {
    projectAccess = ProjectAccess.Update
  }
  def setProjectToReadOnlyMode(): Unit = {
    projectAccess = ProjectAccess.ReadOnly
  }

  def isProjectReadOnly: Boolean = projectAccess == ProjectAccess.ReadOnly
  def isProjectUpdating: Boolean = projectAccess == ProjectAccess.Update
  def isProjectGenerating: Boolean = projectAccess == ProjectAccess.Generate

}
