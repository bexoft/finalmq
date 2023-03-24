var ejs = require('ejs')
var fs = require('fs')
var helper = require(__dirname + '/helper')
var argv = require('minimist')(process.argv.slice(2));

var fileData = argv['input']
var pathOutput = argv['outpath']
var exportMacro = argv['exportmacro']

var fileTemplateCpp = __dirname + '/cpp_cpp.ejs'
var fileTemplateH = __dirname + '/cpp_h.ejs'

var strData = fs.readFileSync(fileData, 'utf8');
var strTemplateCpp = fs.readFileSync(fileTemplateCpp, 'utf8');
var strTemplateH = fs.readFileSync(fileTemplateH, 'utf8');

var data = JSON.parse(strData);

var fileOutputCpp = fileData + '.cpp'
var fileOutputH = fileData + '.h'

if (pathOutput)
{
    var splitFileOutputH   = fileOutputH.split('/')
    var splitFileOutputCpp = fileOutputCpp.split('/')

    fileOutputCpp = pathOutput + '/' + splitFileOutputCpp[splitFileOutputCpp.length - 1]
    fileOutputH   = pathOutput + '/' + splitFileOutputH[splitFileOutputH.length - 1]
}

try
{
    fs.mkdirSync(pathOutput, { recursive: true })
}
catch (err)
{
}

helper.convertTypeId(data)
var options = {data:data, exportMacro:exportMacro, helper:helper, fileOutputH:fileOutputH}
var strCpp = ejs.render(strTemplateCpp, options)
var strH = ejs.render(strTemplateH, options)

fs.writeFileSync(fileOutputCpp, strCpp, 'utf8');
fs.writeFileSync(fileOutputH, strH, 'utf8');


