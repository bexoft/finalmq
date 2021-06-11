

class FmqEntity
{
    constructor(session, name) 
    {
		this._session = session;
        this._name = name;
		this._id = 0;
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
	
	connect()
	{
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
        this._flagServerDisconnected = true;
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
			if (this._sessionId != '')
			{
				this._sessionDisconnected();
			}
            this._sessionId = sessionId;
			this._sessionConnected();
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

    reply(entityId, correlationId, funcname, inparams)
    {
        if (this._sessionId.length == 0 || !correlationId)
        {
            return;
        } 
        if (correlationId != 0)
        {
            var xmlhttp = this._createRequest();
            xmlhttp.open('POST', this._hostname, true);
            xmlhttp.setRequestHeader('fmq_sessionid', this._sessionId)
			xmlhttp.setRequestHeader('fmq_re_mode', 'MSG_REPLY');
			xmlhttp.setRequestHeader('fmq_re_destid', entityId);
			xmlhttp.setRequestHeader('fmq_re_corrid', correlationId);
			var payload = this._createPayload(inparams);
            xmlhttp.send(payload);
        }
    }

    replyStatus(entityId, correlationId, status)
    {
        if (this._sessionId.length == 0 || !correlationId)
        {
            return;
        } 
        if (correlationId != 0)
        {
            var xmlhttp = this._createRequest();
            xmlhttp.open('POST', this._hostname, true);
            xmlhttp.setRequestHeader('fmq_sessionid', this._sessionId)
			xmlhttp.setRequestHeader('fmq_re_mode', 'MSG_REPLY');
			xmlhttp.setRequestHeader('fmq_re_destid', entityId);
			xmlhttp.setRequestHeader('fmq_re_corrid', correlationId);
			xmlhttp.setRequestHeader('fmq_re_status', status);
            xmlhttp.send('');
        }
    }

    createSession(funcresult)
    {
		this.requestReply('fmq', 'createsession', null, function(outparams, context) {
			context.funcresult(outparams);
			context._this._longpoll();
		}, {_this:this, funcresult:funcresult});
    }
	
    removeSession()
    {
		this.requestReply('fmq', 'removesession');
    }

	createEntity(name, entityClass)
	{
		var entity = null;
		if (entityClass)
		{
			entity = new entityClass(this, name);
		}
		else
		{
			entity = new FmqEntity(this, name);
		}
		this._entities.push(entity);
		entity.connect();
		return entity;
	}
	
	_sessionDisconnected()
	{
		for (var i = 0; i < this._entities.length; i++)
		{
			if (this._entities[i].sessionDisconnected)
			{
				this._entities[i].sessionDisconnected();
			}
		}
	}
	
	_sessionConnected()
	{
		for (var i = 0; i < this._entities.length; i++)
		{
			if (this._entities[i].sessionConnected)
			{
				this._entities[i].sessionConnected();
			}
			this._entities[i].connect();
		}
	}
	
	_serverDisconnected()
	{
		for (var i = 0; i < this._entities.length; i++)
		{
			if (this._entities[i].serverDisconnected)
			{
				this._entities[i].serverDisconnected();
			}
		}
	}
	
	_serverConnected()
	{
		for (var i = 0; i < this._entities.length; i++)
		{
			if (this._entities[i].serverConnected)
			{
				this._entities[i].serverConnected();
			}
		}
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

					if (!err && xmlhttp._this._flagServerDisconnected)
					{
						xmlhttp._this._serverConnected();
					}

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
							if (entity && entity[methodName])
							{
                                entity[methodName](header.corrid, params);
							}
							else
							{
                                xmlhttp._this.replyStatus(header.srcid, header.corrid, 'STATUS_REQUEST_NOT_FOUND');
							}
                        }
                    }
                }
                else
                {
                    err = true;
                    setTimeout(function (_this) { _this._longpoll(); }, 5000, xmlhttp._this);
                }

                if (err && !xmlhttp._this._flagServerDisconnected)
                {
                    xmlhttp._this._serverDisconnected();
                }
                xmlhttp._this._flagServerDisconnected = err;
            }
        }
		
		var path = this._hostname + '/fmq/longpoll'
        if (this._flagServerDisconnected)
		{
			path += '?timeout=0';
		}
		else
		{
			path += '?timeout=20000';
		}
		
        xmlhttp.open("POST", path, true);
        xmlhttp.setRequestHeader('fmq_sessionid', this._sessionId);
        xmlhttp.send('');
    }
}






