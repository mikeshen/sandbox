document.addEventListener('DOMContentLoaded', () => {
  const bootstrapScript = document.querySelector('script[src*="bootstrap"]');
  
  function initializeAnalysisModal() {
    try {
      // Check if Bootstrap is loaded
      if (typeof bootstrap === 'undefined') {
        console.error('Bootstrap is not loaded');
        return;
      }
      
      // Check for required elements
      const analysisDataEl = document.getElementById('analysis-data');
      const analysisContentEl = document.getElementById('analysis-content');
      const analysisModalEl = document.getElementById('analysisModal');
      const showAnalysisBtn = document.getElementById('show-analysis-btn');
      
      if (!analysisDataEl || !analysisContentEl || !analysisModalEl || !showAnalysisBtn) {
        console.error('One or more required elements are missing');
        return;
      }
      
      // Get analysis text from the data element
      let analysisText;
      try {
        analysisText = JSON.parse(analysisDataEl.textContent);
      } catch (e) {
        console.error('Failed to parse analysis data:', e);
        analysisContentEl.innerHTML = '<div class="alert alert-danger">Failed to load analysis data.</div>';
        return;
      }
      
      // Use marked.js to parse the markdown
      if (typeof marked === 'undefined') {
        console.error('Marked.js is not loaded');
        analysisContentEl.innerHTML = '<div class="alert alert-danger">Markdown parser not available.</div>';
        return;
      }
      
      const parsedContent = marked.parse(analysisText);
      
      // Prepare content in the modal
      analysisContentEl.innerHTML = parsedContent;
      
      // Initialize the modal
      const analysisModal = new bootstrap.Modal(analysisModalEl);
      
      // Add click event for the button
      showAnalysisBtn.addEventListener('click', () => {
        analysisModal.show();
      });
      
      console.log('Analysis modal initialized successfully');
    } catch (err) {
      console.error('Error initializing analysis modal:', err);
    }
  }
  
  // If script already loaded, initialize immediately
  if (typeof bootstrap !== 'undefined') {
    initializeAnalysisModal();
  } else if (bootstrapScript) {
    // Otherwise wait for it to load
    bootstrapScript.addEventListener('load', initializeAnalysisModal);
  } else {
    console.error('Bootstrap script not found in the document');
  }
});