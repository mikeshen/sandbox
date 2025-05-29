// filepath: /html-to-pdf-converter/html-to-pdf-converter/src/convert.js
const fs = require('fs');
const path = require('path');
const puppeteer = require('puppeteer');

const htmlDir = path.join(__dirname, '../html');
const outputDir = path.join(__dirname, '../output');

async function convertHtmlToPdf(htmlFileName) {
    const htmlFilePath = path.join(htmlDir, htmlFileName);
    const pdfFilePath = path.join(outputDir, htmlFileName.replace('.html', '.pdf'));

    if (!fs.existsSync(htmlFilePath)) {
        throw new Error(`HTML file not found: ${htmlFilePath}`);
    }

    const browser = await puppeteer.launch();
    const page = await browser.newPage();
    
    await page.goto(`file://${htmlFilePath}`, { waitUntil: 'networkidle0' });
    await page.pdf({ path: pdfFilePath, format: 'A4' });

    await browser.close();
    console.log(`Converted ${htmlFileName} to PDF: ${pdfFilePath}`);
}

async function convertAllHtmlFiles() {
    const files = fs.readdirSync(htmlDir).filter(file => file.endsWith('.html'));

    for (const file of files) {
        await convertHtmlToPdf(file);
    }
}

convertAllHtmlFiles().catch(error => {
    console.error('Error during conversion:', error);
});