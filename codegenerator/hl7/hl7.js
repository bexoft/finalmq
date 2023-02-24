var ejs = require('ejs')
var fs = require('fs')
var helper = require(__dirname + '/helper')
var argv = require('minimist')(process.argv.slice(2));

var fileData = argv['input']
var pathOutput = argv['outpath']

var hl7dictionary = require(fileData)

var fileTemplate = __dirname + '/hl7_fmq.ejs'

var strTemplate = fs.readFileSync(fileTemplate, 'utf8');

var fileOutput = fileData + '.fmq'

if (pathOutput)
{
    var splitFileOutput   = fileOutput.split('/')

    fileOutput   = pathOutput + '/' + splitFileOutput[splitFileOutput.length - 1]
}

helper.makeFieldNames(hl7dictionary);
var options = {helper:helper, hl7dictionary:hl7dictionary, fileOutput:fileOutput}
var str = ejs.render(strTemplate, options)

fs.writeFileSync(fileOutput, str, 'utf8');


