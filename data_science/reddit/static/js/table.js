document.addEventListener('DOMContentLoaded', () => {
  const tableData = JSON.parse(document.getElementById('table-data').textContent);
  const cols = JSON.parse(document.getElementById('table-columns').textContent);

  // build Tabulator column defs
  const columnDefs = cols.map(name => ({
    title: name,
    field: name,
    headerFilter: "input",
    formatter: (name === "Body" || name === "ParentBody") 
      ? "html"
      : undefined,
  }));

  // initialize Tabulator
  new Tabulator("#table", {
    data: tableData,
    columns: columnDefs,
    theme: "bootstrap5",         // use bootstrap theme
    layout: "fitColumns",
    pagination: "local",
    paginationSize: 10,
    movableColumns: true,
    resizableRows: true,
    placeholder: "No comments to display",
  });
});