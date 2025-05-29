# HTML to PDF Converter

This project provides a simple way to convert HTML files or live documentation sites into PDF documents programmatically. It utilizes Puppeteer to handle the rendering and conversion process, and supports both static HTML conversion and crawling live documentation sites to save all pages as PDFs.

## Project Structure

```
html-to-pdf-converter
├── src
│   ├── convert.js        # Main logic for converting local HTML files to PDF
│   ├── crawl-and-pdf.js  # Script for crawling a live site and saving all pages as PDFs
│   └── utils.js          # Utility functions for file handling
├── html                  # Directory for HTML files to convert
│   └── [your-html-files-here]
├── output                # Directory for generated PDF files
│   └── [generated-pdfs-here]
├── package.json          # NPM configuration file
└── README.md             # Project documentation
```

## Installation

1. Clone the repository:

   ```sh
   git clone [repository-url]
   cd html-to-pdf-converter
   ```

2. Install the required dependencies:

   ```sh
   npm install
   ```

## Usage

### Convert Local HTML Files

1. Place your HTML files in the `html` directory.

2. Run the conversion script:

   ```sh
   node src/convert.js
   ```

3. The generated PDF files will be saved in the `output` directory.

### Crawl and Convert a Live Documentation Site

1. Edit `src/crawl-and-pdf.js` and set the `BASE_URL` to your documentation's starting page.

2. Run the crawler script:

   ```sh
   node src/crawl-and-pdf.js
   ```

3. The script will crawl all reachable links (including sidebar navigation) from the base URL, saving each page as a PDF in the `output` directory.

## Dependencies

This project uses the following libraries:
- Puppeteer: A Node library which provides a high-level API to control Chrome or Chromium over the DevTools Protocol.

## Contributing

Feel free to submit issues or pull requests if you have suggestions or improvements for the project.

## License

This project is licensed under the MIT License. See the LICENSE file for details.