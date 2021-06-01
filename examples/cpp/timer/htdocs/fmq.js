

class FmqEntity
{
    constructor(session, name) 
    {
		this._session = session;
        this._name = name;
		this._id = 0;
		this._callback = null;
    }
	
    requestReply(funcname, inparams, funcresult, context)
    {
		return this._session.requestReply(this._name, funcname, inparams, funcresult, context)
	}

    sendEvent(funcname, inparams)
    {
		return this._session.sendEvent(this._name, funcname, inparams)
    }

    reply(correlationId, funcname, inparams)
    {
		return this._session.sendEvent(correlationId, funcname, inparams)
    }
	
	connect(callback)
	{
		this._callback = callback;
		this.requestReply('finalmq.remoteentity.ConnectEntity', null, function(outparams, context) {
			if (outparams.fmqheader.status == 'STATUS_OK')
			{
				context._this._id = outparams.fmqheader.srcid;
			}
		}, {_this:this});
	}
}



class FmqSession
{
    constructor(hostname) 
    {
        this._hostname = '';
        this._sessionId = '';
        this._serverDisconnected = true;
        this._entities = [];
        if (hostname)
        {
            this._setHostname(hostname);
        }
    }

    _setHostname(hostname)
    {
        this._hostname = hostname;
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
	
	_getEntity(id)
	{
		for (var i = 0; i < this._entities.length; i++)
		{
			if (this._entities[i]._id == id)
			{
				return this._entities[i];
			}
		}
		return null;
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
	            if (xmlhttp.readyState == 4)
	            {                                                                           
                    xmlhttp._this._updateSessionId(xmlhttp);
	                var header = xmlhttp._this._header(xmlhttp);
	                var params = xmlhttp._this._getParams(xmlhttp.responseText);
					params.fmqheader = header;
					params.httpstatus = xmlhttp.status;
	                xmlhttp.funcresult(params, xmlhttp._context);
	            }
	        }
        }
        xmlhttp.open('POST', this._hostname + '/' + objectname + '/' + funcname, (funcresult == null) ? false : true);
        xmlhttp.setRequestHeader('fmq_sessionid', this._sessionId);
		var payload = this._createPayload(inparams);
        xmlhttp.send(payload);
	    if (funcresult == null)
	    {
            this._updateSessionId(xmlhttp);
			var header = this._header(xmlhttp);
            var params = xmlhttp._this._getParams(xmlhttp.responseText);
			params.fmqheader = header;
			params.httpstatus = xmlhttp.status;
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
        xmlhttp.open('POST', this._hostname + '/' + objectname + '/' + funcname, true);
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
            xmlhttp.open('POST', this._hostname + '/' + objectname + '/' + funcname, true);
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
            xmlhttp.open('POST', this._hostname + '/' + objectname + '/' + funcname, true);
            xmlhttp.setRequestHeader('fmq_sessionid', this._sessionId)
			xmlhttp.setRequestHeader('fmq_re_mode', 'MSG_REPLY');
			xmlhttp.setRequestHeader('fmq_re_corrid', correlationId);
			xmlhttp.setRequestHeader('fmq_re_status', status);
            xmlhttp.send('');
        }
    }

    createSession(funcresult)
    {
		this.requestReply('fmq', 'ping', null, function(outparams, context) {
			context.funcresult(outparams);
			context._this._longpoll();
		}, {_this:this, funcresult:funcresult});
    }
	
    removeSession(funcresult)
    {
		this.requestReply('fmq', 'removesession');
    }

	createEntity(name)
	{
		var entity = new FmqEntity(this, name);
		this._entities.push(entity);
		return entity;
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

                    var responses = xmlhttp.responseText.split(']\t');
                    for (var i = 0; i < responses.length; ++i)
                    {
                        var response = responses[i];
                        if (response.length > 0)
                        {
							response += ']';
							var command = xmlhttp._this._getParams(response);
							var header = command[0];
							var params = command[1];
							params.fmqheader = header;
							params.httpstatus = xmlhttp.status;
                            var methodName = header.type.replace(/\./g, '_');  // replace all '.' by '_'
							var entity = xmlhttp._this._getEntity(header.srcid);
							if (entity._callback && entity._callback[methodName])
							{
                                entity._callback[methodName](header.corrid, params);
							}
							else
							{
                                xmlhttp._this.replyStatus(header.corrid, 'STATUS_REQUEST_NOT_FOUND');
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
        xmlhttp.open("POST", this._hostname + '/fmq/longpoll', true);
        xmlhttp.setRequestHeader('fmq_sessionid', this._sessionId);
        xmlhttp.send('');
		
/*
        if (this._serverDisconnected)
        {
            xmlhttp.send('longpoll=0');
        }
        else
        {
            xmlhttp.send('longpoll=20');
        }
*/
    }
}






