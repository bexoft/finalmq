

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
        this._hostname += '/';
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

    _getParams(responseText)
    {
		if (responseText != '')
		{
			try {
				return eval('(' + responseText + ')');
			}
			finally {
			}
		}
		
		return {responseText:responseText};
    }

    _updateSessionId(xmlhttp)
    {
        var sessionId = xmlhttp.getResponseHeader('fmq_setsession');
        if (sessionId)
        {
            this._sessionId = sessionId;
        }
    }

	_createPayload(inparams)
	{
        var payload = (inparams == null) ? "{}" : JSON.stringify(inparams);
		return payload;
	}
	
	_header(xmlhttp)
	{
		var header = {mode: xmlhttp.getResponseHeader('fmq_re_mode'),
					  corrid: xmlhttp.getResponseHeader('fmq_re_corrid'),
					  status: xmlhttp.getResponseHeader('fmq_re_status'),
					  type: xmlhttp.getResponseHeader('fmq_re_type'),
					  srcid: xmlhttp.getResponseHeader('fmq_re_srcid')};
		return header;
	}

    requestReply(objectname, funcname, inparams, funcresult, context)
    {
        if (this._sessionId.length == 0 && objectname != 'fmq')
        {
            return;
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
	                var header = xmlhttp._this._header(xmlhttp);
	                var params = xmlhttp._this._getParams(xmlhttp.responseText);
					params.replystatus = header.status;
					params.replytype = header.type;
	                xmlhttp.funcresult(params, xmlhttp._context);
	            }
	        }
        }
        xmlhttp.open('POST', this._hostname + objectname + '/' + funcname, (funcresult == null) ? false : true);
        xmlhttp.setRequestHeader('fmq_sessionid', this._sessionId);
		var payload = this._createPayload(inparams);
        xmlhttp.send(payload);
	    if (funcresult == null)
	    {
            this._updateSessionId(xmlhttp);
			var header = this._header(xmlhttp);
            var params = xmlhttp._this._getParams(xmlhttp.responseText);
			params.replystatus = header.status;
			params.replytype = header.type;
	        return params;
	    }
    }

    sendEvent(inparams)
    {
        if (this._sessionId.length == 0)
        {
            return;
        } 
        var xmlhttp = this._createRequest();
        xmlhttp.open('POST', this._hostname + objectname + '/' + funcname, true);
        xmlhttp.setRequestHeader('fmq_sessionid', this._sessionId)
		var payload = this._createPayload(inparams);
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
            xmlhttp.open('POST', this._hostname + objectname + '/' + funcname, true);
            xmlhttp.setRequestHeader('fmq_sessionid', this._sessionId)
			xmlhttp.setRequestHeader('fmq_re_mode', 'MSG_REPLY');
			xmlhttp.setRequestHeader('fmq_re_corrid', correlationId);
			var payload = this._createPayload(inparams);
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
            xmlhttp.setRequestHeader('fmq_sessionid', this._sessionId)
			xmlhttp.setRequestHeader('fmq_re_mode', 'MSG_REPLY');
			xmlhttp.setRequestHeader('fmq_re_corrid', correlationId);
			xmlhttp.setRequestHeader('fmq_re_status', status);
            xmlhttp.send('');
        }
    }

    createSession(funcresult)
    {
		this.requestReply('fmq', 'ping', null, funcresult);
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
        xmlhttp.setRequestHeader('fmq_sessionid', this._sessionId)
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






