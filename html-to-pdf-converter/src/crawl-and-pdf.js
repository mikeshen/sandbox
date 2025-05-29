const puppeteer = require('puppeteer');
const fs = require('fs');
const path = require('path');

// Set the base URL to crawl
const BASE_URL = 'https://docs.prequel.co/docs/'; // Change this to the desired starting URL
const OUTPUT_DIR = path.resolve(__dirname, '../output');

// Set to limit crawl depth and avoid infinite loops
const MAX_DEPTH = 3;

const visited = new Set();

async function crawl(page, url, depth = 1) {
  if (visited.has(url) || depth > MAX_DEPTH) return;
  visited.add(url);
  console.log(`Crawling: ${url}`);

  try {
    await page.goto(url, { waitUntil: 'networkidle2', timeout: 30000 });
    // Save PDF
    const safeName = url.replace(/[^a-z0-9]/gi, '_').toLowerCase();
    const pdfPath = path.join(OUTPUT_DIR, `${safeName}.pdf`);
    await page.pdf({ path: pdfPath, format: 'A4' });
    console.log(`Saved PDF: ${pdfPath}`);

    // Extract all links on the page (all <a> tags)
    const links = await page.evaluate(() => {
      return Array.from(document.querySelectorAll('a')).map(a => a.href);
    });
    for (const link of links) {
      // Only crawl links within the same domain
      if (link.startsWith(BASE_URL)) {
        await crawl(page, link, depth + 1);
      }
    }
  } catch (err) {
    console.error(`Failed to crawl ${url}:`, err.message);
  }
}

(async () => {
  if (!fs.existsSync(OUTPUT_DIR)) {
    fs.mkdirSync(OUTPUT_DIR, { recursive: true });
  }
  const browser = await puppeteer.launch();
  const page = await browser.newPage();
  await crawl(page, BASE_URL);
  await browser.close();
  console.log('Crawling complete.');
})();
