package overlord

abstract class BoardFeature(val name : String, val isHard : Boolean)
{
  var used = false

  def use() : Unit = used = true

}