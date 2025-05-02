document.addEventListener('DOMContentLoaded', () => {
  // Get analysis text from the data element
  const analysisText = JSON.parse(document.getElementById('analysis-data').textContent);
  
  // Use marked.js to parse the markdown
  const parsedContent = marked.parse(analysisText);
  
  // Prepare content in the modal
  document.getElementById('analysis-content').innerHTML = parsedContent;
  
  // Initialize the modal
  const analysisModal = new bootstrap.Modal(document.getElementById('analysisModal'));
  
  // Add click event for the button
  document.getElementById('show-analysis-btn').addEventListener('click', () => {
    analysisModal.show();
  });
});