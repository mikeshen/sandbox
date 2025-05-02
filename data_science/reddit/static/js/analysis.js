document.addEventListener('DOMContentLoaded', () => {
  // Get analysis text from the data element
  const analysisText = JSON.parse(document.getElementById('analysis-data').textContent);
  
  // Use marked.js to parse the markdown and display it
  document.getElementById('analysis-content').innerHTML = marked.parse(analysisText);
});