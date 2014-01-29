package com.stripe.ctf.instantcodesearch

import java.io._
import java.util.Arrays
import java.nio.file._
import java.nio.charset._
import java.nio.file.attribute.BasicFileAttributes

class Indexer(indexId: Int) {
  val idx = new Index()

  def index(path: String) : Indexer = {
    val root = FileSystems.getDefault().getPath(path)

    Files.walkFileTree(root, new SimpleFileVisitor[Path] {
      override def preVisitDirectory(dir : Path, attrs : BasicFileAttributes) : FileVisitResult = {
        if (Files.isHidden(dir) && dir.toString != ".")
          return FileVisitResult.SKIP_SUBTREE
        return FileVisitResult.CONTINUE
      }
      override def visitFile(file : Path, attrs : BasicFileAttributes) : FileVisitResult = {
        if (Files.isHidden(file))
          return FileVisitResult.CONTINUE
        if (!Files.isRegularFile(file, LinkOption.NOFOLLOW_LINKS))
          return FileVisitResult.CONTINUE
        if (Files.size(file) > (1 << 20))
          return FileVisitResult.CONTINUE
        if(Math.abs(file.toString().hashCode() % 3) != (indexId-1)){
          return FileVisitResult.CONTINUE
        }
        val bytes = Files.readAllBytes(file)
        if (Arrays.asList(bytes).indexOf(0) > 0)
          return FileVisitResult.CONTINUE
        val decoder = Charset.forName("UTF-8").newDecoder()
        decoder onMalformedInput CodingErrorAction.REPORT
        decoder onUnmappableCharacter CodingErrorAction.REPORT
        try {
          val r = new InputStreamReader(new ByteArrayInputStream(bytes), decoder)
          val strContents = slurp(r)
          idx.addFile(root.relativize(file).toString, strContents)
        } catch {
          case e: IOException => {
            return FileVisitResult.CONTINUE
          }
        }

        return FileVisitResult.CONTINUE
      }
    })

    return this
  }
}
