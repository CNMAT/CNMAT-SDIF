#!/usr/local/bin/tclsh8.0


array set sspMsgProcArray {
    index	sspServer_IndexProc
    seek_to	sspServer_SeekToProc
}


load $env(SDIF_LIB)/libsdifu_tcl.so
socket -server sspServerProc 7343


proc sspServerProc { sockd addr port } {

    global sspMsgProcArray

    puts stderr "ssp server: accepted client connection from ${addr}:$port."

    fconfigure $sockd -buffering line
    set ssp_msg [gets $sockd]
    set msg_list [split $ssp_msg { }]
    if { [catch { set sspMsgProcArray([lindex $msg_list 0]) }] } {
	puts $sockd "unknown message: \"[lindex $msg_list 0]\""
    } else {
	$sspMsgProcArray([lindex $msg_list 0]) $sockd $msg_list
    }
    close $sockd

}


proc sspServer_IndexProc { sockd msg } {

    set filename [lindex $msg 1]
    if { [catch { set index_string [SDIFU_GetIndexStringFromFile $filename] } err_msg] } {
	puts -nonewline $sockd "unable to index \"$filename\": $err_msg"
    } else {
	puts -nonewline $sockd $index_string
    }

    puts stderr "ssp_server.tcl: indexed \"$filename\" for ${addr}:${port}."

}


proc sspServer_SeekToProc { sockd msg } {

    puts $sockd "sspSeekToProc invoked."

}


puts stderr "cnmat sdif stream server v0.1"
puts stderr "by sami khoury (khoury@cnmat.berkeley.edu)"
puts stderr "\nAwaiting client connections..."

vwait x
