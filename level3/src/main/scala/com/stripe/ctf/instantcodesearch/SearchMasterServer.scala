package com.stripe.ctf.instantcodesearch

import com.twitter.util.Future
import org.jboss.netty.util.CharsetUtil.UTF_8
import org.jboss.netty.handler.codec.http._
import org.jboss.netty.buffer.ChannelBuffers.copiedBuffer

class SearchMasterServer(port: Int, id: Int) extends AbstractSearchServer(port, id) {
  val NumNodes = 3

  def this(port: Int) { this(port, 0) }

  val clients = (1 to NumNodes)
    .map { id => new SearchServerClient(port + id, id)}
    .toArray

  override def isIndexed() = {
    val responsesF = Future.collect(clients.map {client => client.isIndexed()})
    val successF = responsesF.map {responses => responses.forall { response =>

        (response.getStatus() == HttpResponseStatus.OK
          && response.getContent.toString(UTF_8).contains("true"))
      }
    }
    successF.map {success =>
      if (success) {
        successResponse()
      } else {
        errorResponse(HttpResponseStatus.BAD_GATEWAY, "Nodes are not indexed")
      }
    }.rescue {
      case ex: Exception => Future.value(
        errorResponse(HttpResponseStatus.BAD_GATEWAY, "Nodes are not indexed")
      )
    }
  }

  override def healthcheck() = {
    val responsesF = Future.collect(clients.map {client => client.healthcheck()})
    val successF = responsesF.map {responses => responses.forall { response =>
        response.getStatus() == HttpResponseStatus.OK
      }
    }
    successF.map {success =>
      if (success) {
        successResponse()
      } else {
        errorResponse(HttpResponseStatus.BAD_GATEWAY, "All nodes are not up")
      }
    }.rescue {
      case ex: Exception => Future.value(
        errorResponse(HttpResponseStatus.BAD_GATEWAY, "All nodes are not up")
      )
    }
  }

  override def index(path: String) = {
    System.err.println(
      "[master] Requesting " + NumNodes + " nodes to index path: " + path
    )

    val responses = Future.collect(clients.map {client => client.index(path)})
    responses.map {_ => successResponse()}
  }

  // TODO: Implement load balancing across clients
  override def query(q: String) = {

    Future
      .collect(clients.map {client => client.query(q)})
      .map{ clientResponses =>
        val content = clientResponses
                 .map{r => r.getContent().toString(UTF_8)}
                 .filterNot {_.isEmpty}
                 .mkString("{\"success\": true,\n \"results\": [", ",\n", "]}")

        val response = new DefaultHttpResponse(HttpVersion.HTTP_1_1, HttpResponseStatus.OK)

        response.setContent(copiedBuffer(content, UTF_8))

        response
      }
  }


}
