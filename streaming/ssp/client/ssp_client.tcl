#!/usr/local/bin/tclsh8.0


set ssp_server [socket localhost 7343]

proc sspClient_IndexProc { sockd filename } {

    puts $sockd "index $filename"
    flush $sockd
    set response [read $sockd]
    return $response

}


# send an index message.
set response [sspClient_IndexProc $ssp_server /usr/local/sdif/trumpet.sdif]
puts -nonewline stderr $response

if 0 {
# send a seek_to message.
set sockd [socket localhost 7343]
fconfigure $sockd -buffering line
puts $sockd "seek_to /usr/local/sdif/trumpet.sdif 1234,0.0"
puts stderr "ssp client: server said: \"[gets $sockd]\""
close $sockd

# send a message we know is not supported.
set sockd [socket localhost 7343]
fconfigure $sockd -buffering line
puts $sockd "unsupported"
puts stderr "ssp client: server said: \"[gets $sockd]\""
close $sockd
}
