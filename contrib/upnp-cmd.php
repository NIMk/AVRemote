#!/usr/bin/php-cgi -q
<?php

# UPnP Command Line Tool for WDTV Live
# Version: 0.1
# Author: Zoster


#### Command Line Functions ####

if ( $argc < 2 ) {
    die (_stringHelp());
}

$actionName = $argv[1];

if ( function_exists('_action' . $actionName)) {
    switch ($argc) {
        case 2:
            call_user_func('_action' . $actionName);
            break;
        case 3:
            call_user_func('_action' . $actionName, $argv[2]);
            break;
        case 4:
            call_user_func('_action' . $actionName, $argv[2], $argv[3]);
            break;
        default:
            call_user_func('_action' . $actionName);
            break;
    }
} else {
    die (_stringHelp());
}

function _stringHelp() {
    $hlp  = 'UPnP command line tool for WDTV Live v0.1' . "\n";
    $hlp .= 'Usage: upnp-cmd action argument' . "\n";
    $hlp .= '' . "\n";
    $hlp .= 'Available AVTransport actions:' . "\n";
    $hlp .= '' . "\n";
    $hlp .= 'GetCurrentTransportActions' . "\n";
    $hlp .= 'GetDeviceCapabilities' . "\n";
    $hlp .= 'GetMediaInfo' . "\n";
    $hlp .= 'GetPositionInfo' . "\n";
    $hlp .= 'GetTransportInfo' . "\n";
    $hlp .= 'GetTransportSettings' . "\n";
    $hlp .= 'Next' . "\n";
    $hlp .= 'Pause' . "\n";
    $hlp .= 'Play' . "\n";
    $hlp .= 'Previous' . "\n";
    $hlp .= 'Seek <SeekMode> <SeekTarget> (allowed SeekMode: "X_DLNA_REL_BYTE", "REL_TIME", "TRACK_NR")' . "\n";
    $hlp .= 'SetAVTransportURI <URI> <URIMetaData> (allowed URI: "http://server/file", "file:///folder/file"'  . "\n";
    $hlp .= 'SetPlayMode <NewPlayMode> (allowed NewPlayMode = "NORMAL", "REPEAT_ONE", "REPEAT_ALL", "RANDOM")' . "\n";
    $hlp .= 'Stop' . "\n";
    $hlp .= 'X_DLNA_GetBytePositionInfo' . "\n";
    $hlp .= '' . "\n";
    $hlp .= 'Available RenderingControl actions:' . "\n";
    $hlp .= '' . "\n";
    $hlp .= 'GetMute' . "\n";
    $hlp .= 'GetVolume' . "\n";
    $hlp .= 'SetMute <DesiredMute> (allowed DesiredMute = 0 or 1)' . "\n";
    $hlp .= 'SetVolume <DesiredVolume> (allowed DesiredVolume = 1 to 100)' . "\n";
    return $hlp;
}


#### General UPnP Functions ####

#### AVTransport Actions ####

function _actionGetCurrentTransportActions() {
    $action = 'GetCurrentTransportActions';
    $args =  '<InstanceID>0</InstanceID>' . "\r\n";
    $result = _sendUPnPCommand($action, $args, 'AVTransport');
}

function _actionGetDeviceCapabilities() {
    $action = 'GetDeviceCapabilities';
    $args =  '<InstanceID>0</InstanceID>' . "\r\n";
    $result = _sendUPnPCommand($action, $args, 'AVTransport');
}

function _actionGetMediaInfo() {
    $action = 'GetMediaInfo';
    $args =  '<InstanceID>0</InstanceID>' . "\r\n";
    $result = _sendUPnPCommand($action, $args, 'AVTransport');
}

function _actionGetPositionInfo() {
    $action = 'GetPositionInfo';
    $args =  '<InstanceID>0</InstanceID>' . "\r\n";
    $result = _sendUPnPCommand($action, $args, 'AVTransport');
}

function _actionGetTransportInfo() {
    $action = 'GetTransportInfo';
    $args =  '<InstanceID>0</InstanceID>' . "\r\n";
    $result = _sendUPnPCommand($action, $args, 'AVTransport');
}

function _actionGetTransportSettings() {
    $action = 'GetTransportSettings';
    $args =  '<InstanceID>0</InstanceID>' . "\r\n";
    $result = _sendUPnPCommand($action, $args, 'AVTransport');
}

function _actionNext() {
    $action = 'Next';
    $args =  '<InstanceID>0</InstanceID>' . "\r\n";
    $result = _sendUPnPCommand($action, $args, 'AVTransport');
}

function _actionPause() {
    $action = 'Pause';
    $args =  '<InstanceID>0</InstanceID>' . "\r\n";
    $result = _sendUPnPCommand($action, $args, 'AVTransport');
}

function _actionPlay($prmSpeed = 1) {
    $action = 'Play';
    $args   = '<InstanceID>0</InstanceID>' . "\r\n";
    $args  .= '<Speed>' . $prmSpeed . '</Speed>' . "\r\n";
    $result = _sendUPnPCommand($action, $args, 'AVTransport');
}

function _actionPrevious() {
    $action = 'Previous';
    $args =  '<InstanceID>0</InstanceID>' . "\r\n";
    $result = _sendUPnPCommand($action, $args, 'AVTransport');
}

function _actionSeek($prmSeekMode, $prmSeekTarget) {
    # SeekModes: "X_DLNA_REL_BYTE" = Bytes Integer, "REL_TIME" = hh:mm:ss, "TRACK_NR" = Track Integer
    $action = 'Seek';
    $args =  '<InstanceID>0</InstanceID>' . "\r\n";
    $args =  '<SeekMode>' . $prmSeekMode . '</SeekMode>' . "\r\n";
    $args =  '<SeekTarget>' . $prmSeekTarget . '</SeekTarget>' . "\r\n";
    $result = _sendUPnPCommand($action, $args, 'AVTransport');
}

function _actionSetAVTransportURI($prmURI = '', $prmMetaData = '') {
    $action = 'SetAVTransportURI';
    $args   = '<InstanceID>0</InstanceID>' . "\r\n";
    $args  .= '<CurrentURI>' . $prmURI . '</CurrentURI>' . "\r\n";
    #$args  .= '<CurrentURIMetaData />'. "\r\n";
    $args  .= '<CurrentURIMetaData>' . htmlentities($prmMetaData) . '</CurrentURIMetaData>' . "\r\n";
    $result = _sendUPnPCommand($action, $args, 'AVTransport');
}

function _actionSetPlayMode($prmNewPlayMode) {
    #allowed NewPlayMode = "NORMAL", "REPEAT_ONE", "REPEAT_ALL", "RANDOM"
    $action = 'SetPlayMode';
    $args =  '<InstanceID>0</InstanceID>' . "\r\n";
    $args  .= '<NewPlayMode>' . $prmNewPlayMode . '</NewPlayMode>' . "\r\n";
    $result = _sendUPnPCommand($action, $args, 'AVTransport');
}

function _actionStop() {
    $action = 'Stop';
    $args =  '<InstanceID>0</InstanceID>' . "\r\n";
    $result = _sendUPnPCommand($action, $args, 'AVTransport');
}

function _actionX_DLNA_GetBytePositionInfo($prmTrackSize) {
    $action = 'X_DLNA_GetBytePositionInfo';
    $args =  '<InstanceID>0</InstanceID>' . "\r\n";
    $args =  '<TrackSize>' . $prmTrackSize . '</TrackSize>' . "\r\n";
    $result = _sendUPnPCommand($action, $args, 'AVTransport');
}

#### END AVTransport Actions ####


#### RenderingControl Actions ####

function _actionGetMute($prmChannel = 'Master') {
    # allowed Channel = "Master", "LF", "RF"
    $action = 'GetMute';
    $args   = '<InstanceID>0</InstanceID>' . "\r\n";
    $args  .= '<Channel>' . $prmChannel . '</Channel>' . "\r\n";
    $result = _sendUPnPCommand($action, $args, 'RenderingControl');
}

function _actionGetVolume($prmChannel = 'Master') {
    $action = 'GetVolume';
    $args   = '<InstanceID>0</InstanceID>' . "\r\n";
    $args  .= '<Channel>' . $prmChannel . '</Channel>' . "\r\n";
    $result = _sendUPnPCommand($action, $args, 'RenderingControl');
}

function _actionSetMute($prmDesiredMute = 1, $prmChannel = 'Master' ) {
    # allowed DesiredMute = 0 or 1
    $action = 'SetMute';
    $args   = '<InstanceID>0</InstanceID>' . "\r\n";
    $args  .= '<Channel>' . $prmChannel . '</Channel>' . "\r\n";
    $args  .= '<DesiredMute>' . $prmDesiredMute . '</DesiredMute>' . "\r\n";
    $result = _sendUPnPCommand($action, $args, 'RenderingControl');
}

function _actionSetVolume($prmDesiredVolume = 1, $prmChannel = 'Master' ) {
    # allowed DesiredVolume = 1 to 100
    $action = 'SetVolume';
    $args   = '<InstanceID>0</InstanceID>' . "\r\n";
    $args  .= '<Channel>' . $prmChannel . '</Channel>' . "\r\n";
    $args  .= '<DesiredVolume>' . $prmDesiredVolume . '</DesiredVolume>' . "\r\n";
    $result = _sendUPnPCommand($action, $args, 'RenderingControl');
}

#### END RenderingControl Actions ####


function _sendUPnPCommand($prmAction, $prmArguments, $prmService) {

    $wdtvPort = _getPort();
    $wdtvIP = _getIP();
    
    $soap  ='<?xml version="1.0" encoding="utf-8"?>' . "\r\n";
    $soap .='<s:Envelope s:encodingStyle="http://schemas.xmlsoap.org/soap/encoding/" xmlns:s="http://schemas.xmlsoap.org/soap/envelope/">' . "\r\n";
    $soap .='   <s:Body>' . "\r\n";
    $soap .='      <u:' . $prmAction . ' xmlns:u="urn:schemas-upnp-org:service:' . $prmService . ':1">' . "\r\n";
    $soap .=            $prmArguments;
    $soap .='      </u:' . $prmAction . '>' . "\r\n";
    $soap .='   </s:Body>' . "\r\n";
    $soap .='</s:Envelope>' . "\r\n";

    $hdr  ='POST /MediaRenderer_' . $prmService . '/control HTTP/1.0' . "\r\n";
    $hdr .='SOAPACTION: "urn:schemas-upnp-org:service:' . $prmService . ':1#' . $prmAction . '"' . "\r\n";
    $hdr .='CONTENT-TYPE: text/xml ; charset="utf-8"' . "\r\n";
    $hdr .='HOST: 127.0.0.1:' . $wdtvPort . "\r\n";
    $hdr .='Connection: close' . "\r\n";
    $hdr .='Content-Length: ' . strlen($soap) . "\r\n";
    $hdr .='' . "\r\n";

    $msg = $hdr . $soap;
 
    $sock = socket_create(AF_INET, SOCK_STREAM, SOL_TCP);
    socket_bind($sock, $wdtvIP);
    socket_connect($sock, '127.0.0.1', $wdtvPort);
    socket_write($sock, $msg);
    $reply = "";
    do {
        $recv = "";
        $recv = socket_read($sock, '1400');
        if($recv != "") {
            $reply .= $recv;
        }
    } while($recv != ""); 
    socket_close($sock);
    $tmpArr = explode("\r\n\r\n", $reply, 2);
    $result = _parseUPnPResponse($tmpArr[1]);
    #print_r($result);
    _printArray($result);
}

function _parseUPnPResponse($prmResponseXML) {
    $doc = new DOMDocument();
    $doc->preserveWhiteSpace = false;
    $doc->formatOutput = true;
    $doc->loadXML($prmResponseXML);
    $respItems = $doc->getElementsByTagName('Body')->item(0)->childNodes->item(0)->childNodes;
    $arrResponse = array();
    foreach ($respItems as $item) {
        $arrResponse[$item->nodeName] = $item->nodeValue;
    }
    return $arrResponse;
}

function _printArray($prmArray) {
    foreach($prmArray as $key => $value) {
        echo '[' . $key . '] => ' . $value . "\n";
    }
}

function _getPort() {
    # get the listening port of DMARender
    $result = exec('lsof -a -i4 -sTCP:LISTEN -c DMARender -F n');
    if ( preg_match('/n\*:(\d*)$/', $result, $matches) ) {
        return (int)$matches[1];
    } else {
        return null;
    }
} # end function

function _getIP() {
    # get our LAN IP
    $result = exec('ipaddr show dev eth0 | grep inet');
    if ( preg_match('/inet ([^\/]*)\//', $result, $matches) ) {
        return $matches[1];
    } else {
        return null;
    }
} # end function

?>

