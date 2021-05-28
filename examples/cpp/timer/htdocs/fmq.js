

class FmqSession
{
    constructor(hostname) 
    {
        this._hostname = '';
        this._sessionId = '';
        this._serverDisconnected = true;
        if (hostname)
        {
            this._setHostname(hostname);
        }
    }

    _setHostname(hostname)
    {
        this._hostname = hostname;
        if (this._hostname != '')
        {
            this._hostname += '/';
        }
    }

    _createRequest()
    {
        var xmlhttp;
        if (window.XMLHttpRequest)                                                      
        {// code for IE7+, Firefox, Chrome, Opera, Safari                             
          xmlhttp = new XMLHttpRequest();                                                 
        }                                                                             
        else                                                                            
        {// code for IE6, IE5                                                         
          xmlhttp = new ActiveXObject("Microsoft.XMLHTTP");                               
        }
	    xmlhttp.withCredentials = true;
        xmlhttp._this = this;
	    return xmlhttp;
    }

    _getCommand(responseText)
    {
        return eval('(' + responseText + ')');
    }


    _updateSessionId(xmlhttp)
    {
        var sessionId = xmlhttp.getResponseHeader('Set-fmqfcgi-Sessionid');
        if (sessionId)
        {
            this._sessionId = sessionId;
        }
    }

	_createPayloadRequest(objectname, funcname, inparams)
	{
        var jsonInParams = (inparams == null) ? "{}" : JSON.stringify(inparams);
		var header = {destname:objectname, mode:"MODE_REQUEST", type:funcname};
		var jsonHeader = JSON.stringify(header);
		var payload = '[' + jsonHeader + ',\t' + jsonInParams + ']\t';
		return payload;
	}

	_createPayloadReply(correlationId, funcname, inparams)
	{
        var jsonInParams = (inparams == null) ? "{}" : JSON.stringify(inparams);
		var header = {mode:"MODE_REPLY", corrid:correlationId, type:funcname};
		var jsonHeader = JSON.stringify(header);
		var payload = '[' + jsonHeader + ',\t' + jsonInParams + ']\t';
		return payload;
	}

	_createPayloadReplyStatus(correlationId, status)
	{
		var header = {mode:"MODE_REPLY", corrid:correlationId, status:status};
		var jsonHeader = JSON.stringify(header);
		var payload = '[' + jsonHeader + ',\t{}]\t';
		return payload;
	}

    requestReply(objectname, funcname, inparams, funcresult, context)
    {
        if (this._sessionId.length == 0)
        {
//!!!            return;
        } 
        var xmlhttp = this._createRequest();
        xmlhttp.funcresult = funcresult;
        if (funcresult != null)
        {
	        xmlhttp._context = context;
	        xmlhttp.onreadystatechange = function()
	        {
	            if (xmlhttp.readyState == 4 && xmlhttp.status == 200)                             
	            {                                                                           
                    xmlhttp._this._updateSessionId(xmlhttp);
		        	var command = xmlhttp._this._getCommand(xmlhttp.responseText);
	                var cmd = command[0];
	                var params = command[1];
					if (cmd.status)
					{
						params.replystatus = cmd.status;
					}
					else
					{
						params.replystatus = 'STATUS_OK';
					}
	                xmlhttp.funcresult(params, xmlhttp._context);
	            }
	        }
        }
        xmlhttp.open('POST', this._hostname, (funcresult == null) ? false : true);
        xmlhttp.setRequestHeader('FMQ_SESSIONID', this._sessionId);
		var payload = this._createPayloadRequest(objectname, funcname, inparams);
        xmlhttp.send(payload);
	    if (funcresult == null)
	    {
            this._updateSessionId(xmlhttp);
	        var command = this._getCommand(xmlhttp.responseText);
	        var cmd = command[0];
	        var params = command[1];
            params.replystatus = cmd.status;
	        return params;
	    }
    }

    sendEvent(objectname, funcname, inparams)
    {
        if (this._sessionId.length == 0)
        {
            return;
        } 
        var xmlhttp = this._createRequest();
        xmlhttp.open('POST', this._hostname, true);
        xmlhttp.setRequestHeader('FMQ_SESSIONID', this._sessionId)
		var payload = this._createPayloadRequest(objectname, funcname, inparams);
        xmlhttp.send(payload);
    }

    reply(correlationId, funcname, inparams)
    {
        if (this._sessionId.length == 0)
        {
            return;
        } 
        if (correlationId != 0)
        {
            var xmlhttp = this._createRequest();
            xmlhttp.open('POST', this._hostname, true);
            xmlhttp.setRequestHeader('FMQ_SESSIONID', this._sessionId)
			var payload = this._createPayloadReply(correlationId, funcname, inparams);
            xmlhttp.send(payload);
        }
    }

    replyStatus(correlationId, status)
    {
        if (this._sessionId.length == 0)
        {
            return;
        } 
        if (correlationId != 0)
        {
            var xmlhttp = this._createRequest();
            xmlhttp.open('POST', this._hostname + 'root.fmq', true);
            xmlhttp.setRequestHeader('FMQ_SESSIONID', this._sessionId)
			var payload = this._createPayloadReplyStatus(correlationId, status);
            xmlhttp.send(payload);
        }
    }

    createSession(funcresult)
    {
		this.requestReply('MyService', 'finalmq.remoteentity.ConnectEntity', null, funcresult);
    }
/*
    removeSession(funcresult)
    {
        if (this._sessionId.length == 0)
        {
            return;
        } 
        var xmlhttp = this._createRequest();
        xmlhttp.funcresult = funcresult;
        if (funcresult != null)
        {
            xmlhttp.onreadystatechange = function()
            {
                if (xmlhttp.readyState == 4 && xmlhttp.status == 200)                             
                {                                                                           
                    xmlhttp.funcresult();
                }
            }
        }
        xmlhttp.open('POST', this._hostname + 'root.fmq?removesession', (funcresult == null) ? false : true);
        xmlhttp.setRequestHeader('FMQ_SESSIONID', this._sessionId)
        this._sessionId = '';
        xmlhttp.send('');
    }

    _longpoll()
    {
        if (this._sessionId.length == 0)
        {
            return;
        } 
        var xmlhttp;
        xmlhttp = this._createRequest();
        xmlhttp.onreadystatechange=function()                                           
        {                                                                         
            if (xmlhttp.readyState==4)
            {
                var err = false;
                if (xmlhttp.status==200)
                {
                    xmlhttp._this._updateSessionId(xmlhttp);
                    xmlhttp._this._longpoll();

                    var responses = xmlhttp.responseText.split('\n');
                    for (var i = 0; i < responses.length; ++i)
                    {
                        var response = responses[i];
                        if (response.length > 0)
                        {
                            var command = xmlhttp._this._getCommand(response);                    
	                        var cmd = command[0];
	                        var params = command[1];
                            var methodName = cmd.type.replace(/\./g, '_');  // replace all '.' by '_'
                            if (xmlhttp._this[methodName])
                            {
                                xmlhttp._this[methodName](cmd.src, cmd.corrid, params);
                            }
                            else
                            {
                                xmlhttp._this.replyStatus(cmd.corrid, 'STATUS_REQUEST_NOT_FOUND');
                            }
                        }
                    }
                }
                else
                {
                    err = true;
                    setTimeout(function (_this) { _this._longpoll(); }, 5000, xmlhttp._this);
                }

                if (err && !xmlhttp._this._serverDisconnected)
                {
                    if (xmlhttp._this.serverdisconnected)
                    {
                        xmlhttp._this.serverdisconnected();
                    }
                }
                if (!err && xmlhttp._this._serverDisconnected)
                {
                    if (xmlhttp._this.serverreconnected)
                    {
                        xmlhttp._this.serverreconnected();
                    }
                }
                xmlhttp._this._serverDisconnected = err;
            }
        }
        xmlhttp.open("POST", this._hostname + 'root.fmq', true);
        xmlhttp.setRequestHeader('FMQ_SESSIONID', this._sessionId)
        if (this._serverDisconnected)
        {
            xmlhttp.send('longpoll=0');
        }
        else
        {
            xmlhttp.send('longpoll=20');
        }
    }
*/
}






