// This file contains utility functions for handling file operations related to HTML to PDF conversion.

const fs = require('fs');
const path = require('path');

// Function to read HTML files from the specified directory
const readHtmlFiles = (directory) => {
    return fs.readdirSync(directory).filter(file => path.extname(file) === '.html').map(file => path.join(directory, file));
};

// Function to write the generated PDF file to the output directory
const writePdfFile = (outputPath, data) => {
    fs.writeFileSync(outputPath, data);
};

module.exports = {
    readHtmlFiles,
    writePdfFile
};