package Overlord

extension (s: String)
  def overlordStringInterpolate(dictionary: Map[String, String]): String = {
    // TODO optimise this as its very inefficient way of doing this
    var r = s
    for (k, v) <- dictionary do r = r.replace(k, v)
    r
  }
