#!/bin/sh
#
# script posted by Sailort
# http://wdtvforum.com/main/index.php?topic=5917.0
#
# Public Domain


LOCKFILE=/tmp/pid.ks
KEYSEQUENCE=/tmp/ks.ks
PBSTATUS=/tmp/pbstatus.ks
CHAPTERFILE=/tmp/chapter_list.tmp
GPOSRESULTFILE=/tmp/gposresult.ks
PBRESULTFILE=/tmp/pbstatusresult.ks
SEEKRESULTFILE=/tmp/seekresult.ks
PLAYRESULTFILE=/tmp/playresult.ks

export RETVALUE
export MEDIADURATION
export CURRENTTIME
export PBSTATUS

SKIPSTEPS=0
SKIPTIMES=60000:300000:900000:1800000:3600000:
SKIPTIMESRANGE=5
SKIPDIRECTION=0
SKIPCHAPTERLAXITY=5000

CleanUp() {
	logger -t KEYHANDLER "CleanUp start"
	[ -f $LOCKFILE ] && rm $LOCKFILE
	[ -f $KEYSEQUENCE ] && rm $KEYSEQUENCE
	logger -t KEYHANDLER "CleanUp end"
}

TimeToMS() {
	if [ "$1" == "0" ]; then return 1
	fi
	hr="$(echo $1 | cut -d: -f1)" 
	mn="$(echo $1 | cut -d: -f2)" 
	sc="$(echo $1 | cut -d: -f3)"
	hr="$(echo ${hr#0})"
	mn="$(echo ${mn#0})"
	sc="$(echo ${sc#0})"
	RETVALUE=$(( ( $hr * 3600 + $mn * 60 + $sc ) * 1000 ))
	return 0
}

MSToTime() {
	tmptime="$(($1/1000))"
	hr="$(($tmptime/3600))"
	mn="$((($tmptime-$hr*3600)/60))"
	sc="$(($tmptime-$hr*3600-$mn*60))"
	RETVALUE=$hr:$mn:$sc
	return 0
}

GetPosition() {
	wget -q --post-data="<?xml version="1.0" encoding="utf-8"?><s:Envelope s:encodingStyle="http://schemas.xmlsoap.org/soap/encoding/" xmlns:s="http://schemas.xmlsoap.org/soap/envelope/"><s:Body><u:GetPositionInfo xmlns:u="urn:schemas-upnp-org:service:AVTransport:1"> <InstanceID>0</InstanceID></u:GetPositionInfo></s:Body></s:Envelope>" $localip:$upnpport/MediaRenderer_AVTransport/control --header='SOAPACTION: "urn:schemas-upnp-org:service:AVTransport:1#GetPositionInfo"' --bind-address=$localip --output-document=$GPOSRESULTFILE

	TimeToMS "$(awk -F '</?RelTime>' 'NF==3{print $2}' $GPOSRESULTFILE)"
	CURRENTTIME=$RETVALUE
 
	TimeToMS "$(awk -F '</?TrackDuration>' 'NF==3{print $2}' $GPOSRESULTFILE)"
	MEDIADURATION=$RETVALUE
	
	logger -t KEYHANDLER "GetPosition: CT[$CURRENTTIME] TD[$MEDIADURATION]"
}

GetPBStatus() {
wget -q --post-data="<?xml version="1.0" encoding="utf-8"?><s:Envelope s:encodingStyle="http://schemas.xmlsoap.org/soap/encoding/" xmlns:s="http://schemas.xmlsoap.org/soap/envelope/"><s:Body><u:GetTransportInfo xmlns:u="urn:schemas-upnp-org:service:AVTransport:1"> <InstanceID>0</InstanceID></u:GetTransportInfo></s:Body></s:Envelope>" $localip:$upnpport/MediaRenderer_AVTransport/control --header='SOAPACTION: "urn:schemas-upnp-org:service:AVTransport:1#GetTransportInfo"' --bind-address=$localip --output-document=$PBRESULTFILE

	PBSTATUS="$(awk -F '</?CurrentTransportState>' 'NF==3{print $2}' $PBRESULTFILE )"
	logger -t KEYHANDLER "GETPBStatus $PBSTATUS"
}

SeekTo() {
wget -q --post-data="<?xml version="1.0" encoding="utf-8"?><s:Envelope s:encodingStyle="http://schemas.xmlsoap.org/soap/encoding/" xmlns:s="http://schemas.xmlsoap.org/soap/envelope/"><s:Body><u:Seek xmlns:u="urn:schemas-upnp-org:service:AVTransport:1"><InstanceID>0</InstanceID><Unit>REL_TIME</Unit><Target>$1</Target></u:Seek></s:Body></s:Envelope>" $localip:$upnpport/MediaRenderer_AVTransport/control --header='SOAPACTION: "urn:schemas-upnp-org:service:AVTransport:1#Seek"' --bind-address=$localip --output-document=$SEEKRESULTFILE
}

Play() {
wget -q --post-data="<?xml version="1.0" encoding="utf-8"?><s:Envelope s:encodingStyle="http://schemas.xmlsoap.org/soap/encoding/" xmlns:s="http://schemas.xmlsoap.org/soap/envelope/"><s:Body><u:Play xmlns:u="urn:schemas-upnp-org:service:AVTransport:1"><InstanceID>0</InstanceID><Speed>1</Speed></u:Play></s:Body></s:Envelope>" $localip:$upnpport/MediaRenderer_AVTransport/control --header='SOAPACTION: "urn:schemas-upnp-org:service:AVTransport:1#Play"' --bind-address=$localip --output-document=$PLAYRESULTFILE
}

logger -t KEYHANDLER "Received command $1"
upnpport="$(netstat -lp | grep DMARender | grep LISTEN | cut -d* -f2 | cut -d: -f2 | cut -d' ' -f1)"
localip="$(ifconfig  | grep 'inet addr:'| grep -v '127.0.0.1' | cut -d: -f2 | cut -d' ' -f1)"
logger -t KEYHANDLER "UPnP is $localip:$upnpport"

if [ -f $LOCKFILE ]; then
	runningpid="$(cat $LOCKFILE)"
	logger -t KEYHANDLER "Found PID $runningpid"
	kill $runningpid
fi

echo $$ > $LOCKFILE
runningpid="$(cat $LOCKFILE)"
logger -t KEYHANDLER "New PID $runningpid"

# let media play under menu (on RETURN to menu)
if [ "$1" == "BACK" ]; then
	GetPBStatus
	if [ "$PBSTATUS" == "PLAYING" ]; then
		GetPosition
		MSToTime $CURRENTTIME
		NEWTIME=$RETVALUE
		logger -t KEYHANDLER "BACK: Seek to $NEWTIME   CURRENTTIME[$CURRENTTIME] RETVALUE[$RETVALUE] NEWTIME[$NEWTIME]"
		echo T > /tmp/ir_injection
		sleep 1
		SeekTo $NEWTIME
	fi
	CleanUP
	exit 0
fi


case $1 in
	"FWD")  
		logger -t KEYHANDLER "FWD going to sleep..."
		echo F >>$KEYSEQUENCE
		sleep 1
	;;
	"REW")
		logger -t KEYHANDLER "REW going to sleep..."
		echo R >>$KEYSEQUENCE
		sleep 1
	;;
esac

# read current media playback status and info
GetPBStatus
GetPosition

# if there is no playing media we have nothing to do
if [ "$PBSTATUS" != "PLAYING" ]; then
	logger -t KEYHANDLER "No media playing, nothing to do..."
	CleanUP
	exit 0
fi

if [ "$1" == "LISTTOCHAPTER" ]; then
	
	CHINDEX=0
	
	while read inputline
	do
		CHTIME="$(echo $inputline | cut -d: -f1)"
		if [ "$CHTIME" -gt "$CURRENTTIME" ]; then
			CHINDEX="$(( $CHINDEX - 1 ))"
			break
		fi
		CHINDEX="$(( $CHINDEX + 1 ))"
	done < $CHAPTERFILE
	
	echo G > /tmp/ir_injection
	sleep 1
	echo l > /tmp/ir_injection
	sleep 1
	echo n > /tmp/ir_injection
	
	for curstep in `seq 1 $CHINDEX` 
	do
		sleep 1
		echo d > /tmp/ir_injection
	done
	CleanUp
	exit 0
fi

# if skipping chapters
if [ "$1" == "NEXT" -o "$1" == "PREV" ]; then
	
	if [ ! -f $CHAPTERFILE ]; then
		logger -t KEYHANDLER "Chapter file does not exists, skipping file"
		CleanUp
		case "$1" in
			NEXT)
				echo ] > /tmp/ir_injection
			;;
			PREV)
				echo [ > /tmp/ir_injection
			;;
		esac
		exit 0
	fi
	
	CHTMP="$(echo 0)"
	CHPREV="$(echo 0)"
	CHNEXT="$(echo 0)"

	while read inputline
	do
	  CHTIME="$(echo $inputline | cut -d: -f1)"
	logger -t KEYHANDLER "CHAPTERFIND: CHTIME[$CHTIME] CURRENTTIME[$CURRENTTIME]"
		if [ "$CHTIME" -gt "$CURRENTTIME" ]; then
			CHNEXT=$CHTIME
			break
		fi
		CHPREVPREV=$CHPREV
		CHPREV=$CHTIME
		CHTMP=$CHTIME
	logger -t KEYHANDLER "CHAPTERFIND: CHPREVPREV[$CHPREVPREV] CHPREV[$CHPREV] CHTMP[$CHTMP]"
	done < $CHAPTERFILE

	logger -t KEYHANDLER "Skip direction $1 CHPREV[$CHPREV] CHNEXT[$CHNEXT]"
	
	case "$1" in
		NEXT)
			NEWTIME=$CHNEXT
			if [ "$NEWTIME" == "0" ]; then
				logger -t KEYHANDLER "We are on last chapter, skipping to next file"
				echo ] > /tmp/ir_injection
				exit 1
			fi
			;;
		PREV)
			if [ "$(( $CHPREV + $SKIPCHAPTERLAXITY ))" -gt "$CURRENTTIME" ]; then
				NEWTIME=$CHPREVPREV
			else
				NEWTIME=$CHPREV
			fi
			if [ "$NEWTIME" == "0" ]; then
				logger -t KEYHANDLER "We are on first chapter, skipping to previous file"
				echo [ > /tmp/ir_injection
				exit 1
			fi
			;;
	esac
	
	logger -t KEYHANDLER "Gonna seek to $NEWTIME"
	MSToTime $NEWTIME
	logger -t KEYHANDLER "Gonna seek to $RETVALUE"
	NEWTIME=$RETVALUE
	SeekTo $NEWTIME

	CleanUp
	exit 0
fi

# if moving in file
if [ "$1" == "FWD" -o "$1" == "REW" ]; then
	# calculate "throw"
	while read KEY
	do
		if [ "$KEY" == "F" ] ; then SKIPSTEPS="$(( $SKIPSTEPS+1 ))"
		fi
		if [ "$KEY" == "R" ] ; then SKIPSTEPS="$(( $SKIPSTEPS-1 ))" 
		fi
	done < $KEYSEQUENCE

	# check "throw" direction and multiplicator
	if   [ "$SKIPSTEPS" -eq 0 ] ; then
			SKIPDIRECTION=0
	elif [ "$SKIPSTEPS" -lt 0 ] ; then
			SKIPDIRECTION=-1
			SKIPSTEPS="$(($SKIPSTEPS * -1))"
	elif [ "$SKIPSTEPS" -gt 0 ] ; then
			SKIPDIRECTION=1
	fi
	
	logger -t KEYHANDLER "SKIPDIRECTION[$SKIPDIRECTION] SKIPSTEPS[$SKIPSTEPS]"
	curskip=1
	TMPTIME=$CURRENTTIME
	NEWTIME=0
	
	# if we are within accepted values, and are beyond start and before end of media, we will try to find largest possible "throw"
	for curskip in `seq 1 $SKIPSTEPS`
	do
		SKIPTIME=`echo $SKIPTIMES | cut -d: -f$curskip`
		TMPTIME="$(( $CURRENTTIME + ( $SKIPDIRECTION * $SKIPTIME )))"
		logger -t KEYHANDLER "FINDTHROW curskip[$curskip] skiptime[$SKIPTIME] tmptime[$TMPTIME] newtime[$NEWTIME] currenttime[$CURRENTTIME]"
		if [ "$curskip" -gt "$SKIPTIMESRANGE" -o "$TMPTIME" -le "0" -o "$TMPTIME" -gt "$MEDIADURATION" ] ; then
			break
		fi
		NEWTIME=$TMPTIME
	done
	
	MSToTime $NEWTIME
	NEWTIME=$RETVALUE

	SeekTo $NEWTIME

	CleanUp
	exit 0
fi