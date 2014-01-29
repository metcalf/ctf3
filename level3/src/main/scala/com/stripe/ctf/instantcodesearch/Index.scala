package com.stripe.ctf.instantcodesearch

import java.io._

class Index() extends Serializable {
  var files = collection.mutable.Map[String,String]()

  def addFile(file: String, text: String) {
    files.put(file, text)
  }
}
