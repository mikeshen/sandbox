document.addEventListener("DOMContentLoaded", () => {
  const tableData = JSON.parse(
    document.getElementById("table-data").textContent
  );
  const cols = JSON.parse(document.getElementById("table-columns").textContent);

  // Build Tabulator column defs
  const columnDefs = cols.map((name) => ({
    title: name,
    field: name,
    headerFilter: "input",
    formatter: name === "Body" || name === "ParentContent" ? "html" : undefined,
  }));

  // Initialize Tabulator
  const table = new Tabulator("#table", {
    data: tableData,
    columns: columnDefs,
    theme: "bootstrap5", // use bootstrap theme
    layout: "fitColumns",
    pagination: "local",
    paginationSize: 10,
    movableColumns: true,
    resizableRows: true,
    placeholder: "No comments to display",
    selectable: 1, // Allow row selection
  });

  // Add row click event
  table.on("rowClick", function (e, row) {
    const rowData = row.getData();

    // Get modal elements
    const modal = new bootstrap.Modal(
      document.getElementById("commentDetailModal")
    );

    // Set content
    document.getElementById("detail-subreddit").textContent = rowData.Subreddit;
    document.getElementById("detail-body").innerHTML = marked.parse(
      rowData.Body
    );
    document.getElementById("detail-score").textContent = rowData.Score;
    document.getElementById("detail-source").textContent =
      rowData.Source || "Unknown";

    // Set permalink
    const permalinkElem = document.getElementById("detail-permalink");
    if (rowData.Permalink) {
      permalinkElem.href = `https://www.reddit.com${rowData.Permalink}`;
      permalinkElem.style.display = "inline-block";
    } else {
      permalinkElem.style.display = "none";
    }

    // Handle parent content if exists
    const parentElem = document.getElementById("detail-parent");
    if (rowData.ParentContent) {
      parentElem.innerHTML = marked.parse(rowData.ParentContent);
      parentElem.parentElement.style.display = "block";
    } else {
      parentElem.innerHTML = "<em>No parent content</em>";
      parentElem.parentElement.style.display = "block";
    }

    // Show modal
    modal.show();
  });
});
