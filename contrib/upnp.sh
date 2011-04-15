#!/bin/sh

# small upnp player in shell script, for WDLXTV
# this is supposed to work faster than the upnp-cmd
# needed to lower latency of operation in hdsync.app.bin

# (C) 2011 by Jaromil - GNU GPL v3

if [ -z $1 ]; then
    echo "usage: $0 [command]"
    echo "commands: load filename, play, stop, pause"
    exit 0
fi

HOST=127.0.0.1

# detect UPNP port
UPNPPORT="`lsof -a -i4 -sTCP:LISTEN -c DMARender -F n | awk -v FS=':' '/^n/ {print $2}'`"
if ! [ $UPNPPORT ]; then
    echo "error: no UPNP daemon found."
    exit 1
else
    echo "UPNP daemon found listening on port $UPNPPORT"
fi
PORT=$UPNPPORT


send_message() {
    if [ -z $1 ]; then
	echo "error: send_message"
	return
    fi
    action="$1"
    message="$2"
    smsg="<?xml version=\"1.0\" encoding=\"utf-8\"?><s:Envelope s:encodingStyle=\"http://schemas.xmlsoap.org/soap/encoding/\" xmlns:s=\"http://schemas.xmlsoap.org/soap/envelope/\"><s:Body><u:$action xmlns:u=\"urn:schemas-upnp-org:service:AVTransport:1\"><InstanceID>0</InstanceID>$message</u:$action></s:Body></s:Envelope>"
    wget -O - -q --post-data="$smsg" \
	$HOST:$PORT/MediaRenderer_AVTransport/control \
	--header="SOAPACTION: \"urn:schemas-upnp-org:service:AVTransport:1#$action\"" \
	--bind-address=127.0.0.1
    echo
}

# make cmd case insensitive
cmd="`echo $1 | tr '[:upper:]' '[:lower:]'`"

case $cmd in

    load)
	file="$2"
	# if ! [ -r "$file" ]; then
	#     echo "file not found: $file"
	#     echo "operation aborted."; exit 1
	# fi
	uri="file://$file"
	dirName="`dirname "$file"`"
	fileName="`basename "$file"`"
	fileSize="`stat -c%s "$file"`"

	meta="<DIDL-Lite xmlns=\"urn:schemas-upnp-org:metadata-1-0/DIDL-Lite\" xmlns:dc=\"http://purl.org/dc/elements/1.1/\" xmlns:upnp=\"urn:schemas-upnp-org:metadata-1-0/upnp/\"><item id=\"2$file\" parentID=\"2$dirName\" restricted=\"0\"><dc:title>$fileName</dc:title><dc:date></dc:date><upnp:class>object.item.imageItem</upnp:class><dc:creator></dc:creator><upnp:genre></upnp:genre><upnp:artist></upnp:artist><upnp:album></upnp:album><res protocolInfo=\"file-get:*:*:*:DLNA.ORG_OP=01;DLNA.ORG_CI=0;DLNA.ORG_FLAGS=00000000001000000000000000000000\" protection=\"\" tokenType=\"0\" bitrate=\"0\" duration=\"\" size=\"$fileSize\" colorDepth=\"0\" ifoFileURI=\"\" resolution=\"\">$uri</res></item></DIDL-Lite>"

	msg="<CurrentURI>$uri</CurrentURI><CurrentURIMetaData>$meta</CurrentURIMetaData>"

#	send_message SetAVTransportURI "$msg"

	upnp-cmd SetAVTransportURI "$uri" "$meta"
	# allowed NewPlayMode = "NORMAL", "REPEAT_ONE", "REPEAT_ALL", "RANDOM"
	upnp-cmd SetPlayMode REPEAT_ONE
	;;

    mode)
	mode="$2"
	msg="<NewPlayMode>$mode</NewPlayMode>"
	send_message SetPlayMode "$msg"
	;;

    play)
#	msg="<Speed>1</Speed>"
#	send_message Play "$msg"

	upnp-cmd Play
	;;

    stop)
	upnp-cmd Stop
	;;

    pause)
#	send_message Pause ""
	upnp-cmd Pause
	;;

    *)
	echo "unrecognized command: $cmd"
	exit 1
	;;

esac

#echo "command $cmd executed succesfully"

