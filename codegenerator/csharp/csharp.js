var ejs = require('ejs')
var fs = require('fs')
var helper = require(__dirname + '/helper')
var argv = require('minimist')(process.argv.slice(2));

var fileData = argv['input']
var pathOutput = argv['outpath']

var fileTemplateCS = __dirname + '/csharp_cs.ejs'

var strData = fs.readFileSync(fileData, 'utf8');
var strTemplateCS = fs.readFileSync(fileTemplateCS, 'utf8');

var data = JSON.parse(strData);

var fileOutputCS = fileData + '.cs'

if (pathOutput)
{
    var splitFileOutputCS   = fileOutputCS.split('/')

    fileOutputCS   = pathOutput + '/' + splitFileOutputCS[splitFileOutputCS.length - 1]
}

helper.convertTypeId(data)
var options = {data:data, helper:helper, fileOutputCS:fileOutputCS}
var strCS = ejs.render(strTemplateCS, options)

fs.writeFileSync(fileOutputCS, strCS, 'utf8');


