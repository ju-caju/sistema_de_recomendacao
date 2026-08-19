export type PurchaseRow = { clientCode: string; productCode: string; productName: string };
export type Product = { code: string; name: string };
export type Client = { code: string; purchases: Set<string> };
export type Dataset = { source: string; clients: Client[]; products: Product[]; purchaseCount: number };
export type Recommendation = { product: Product; affinity: number; supporters: number };

export const DEMO_ROWS: PurchaseRow[] = [
  { clientCode: "CL-1042", productCode: "P-101", productName: "Chá verde com hortelã" },
  { clientCode: "CL-1042", productCode: "P-104", productName: "Granola de castanhas" },
  { clientCode: "CL-1087", productCode: "P-101", productName: "Chá verde com hortelã" },
  { clientCode: "CL-1087", productCode: "P-102", productName: "Café especial moído" },
  { clientCode: "CL-1087", productCode: "P-105", productName: "Biscoito de amêndoas" },
  { clientCode: "CL-1120", productCode: "P-104", productName: "Granola de castanhas" },
  { clientCode: "CL-1120", productCode: "P-106", productName: "Mel silvestre" },
  { clientCode: "CL-1158", productCode: "P-101", productName: "Chá verde com hortelã" },
  { clientCode: "CL-1158", productCode: "P-104", productName: "Granola de castanhas" },
  { clientCode: "CL-1158", productCode: "P-107", productName: "Mix de frutas secas" },
  { clientCode: "CL-1193", productCode: "P-102", productName: "Café especial moído" },
  { clientCode: "CL-1193", productCode: "P-103", productName: "Chocolate 70% cacau" },
  { clientCode: "CL-1193", productCode: "P-105", productName: "Biscoito de amêndoas" },
  { clientCode: "CL-1211", productCode: "P-104", productName: "Granola de castanhas" },
  { clientCode: "CL-1211", productCode: "P-106", productName: "Mel silvestre" },
  { clientCode: "CL-1211", productCode: "P-108", productName: "Pasta de amendoim" },
];

function parseCsvRecords(text: string, separator: string) {
  const records: string[][] = [];
  let record: string[] = [];
  let field = "";
  let quoted = false;

  for (let index = 0; index < text.length; index += 1) {
    const character = text[index];
    if (character === '"') {
      if (quoted && text[index + 1] === '"') { field += '"'; index += 1; }
      else quoted = !quoted;
    } else if (character === separator && !quoted) {
      record.push(field.trim()); field = "";
    } else if ((character === "\n" || character === "\r") && !quoted) {
      if (character === "\r" && text[index + 1] === "\n") index += 1;
      record.push(field.trim());
      if (record.some(Boolean)) records.push(record);
      record = []; field = "";
    } else field += character;
  }
  if (field.length > 0 || record.length > 0) {
    record.push(field.trim());
    if (record.some(Boolean)) records.push(record);
  }
  return records;
}

export function parsePurchaseCsv(text: string): PurchaseRow[] {
  const normalizedText = text.replace(/^\uFEFF/, "");
  const firstLine = normalizedText.split(/\r?\n/, 1)[0] ?? "";
  const separator = firstLine.includes(";") ? ";" : ",";
  const records = parseCsvRecords(normalizedText, separator);
  if (records.length < 2 || records[0].length < 4) {
    throw new Error("O CSV deve ter cabeçalho e pelo menos quatro colunas.");
  }
  const rows = records.slice(1).flatMap((fields) => {
    if (fields.length < 4 || !fields[1] || !fields[2]) return [];
    return [{ clientCode: fields[1], productCode: fields[2], productName: fields.slice(3).join(separator).trim() || fields[2] }];
  });
  if (rows.length === 0) throw new Error("O CSV não contém compras válidas.");
  return rows;
}

export function buildDataset(rows: PurchaseRow[], source: string): Dataset {
  const clientMap = new Map<string, Set<string>>();
  const productMap = new Map<string, Product>();
  rows.forEach((row) => {
    if (!clientMap.has(row.clientCode)) clientMap.set(row.clientCode, new Set());
    clientMap.get(row.clientCode)?.add(row.productCode);
    if (!productMap.has(row.productCode)) productMap.set(row.productCode, { code: row.productCode, name: row.productName });
  });
  const clients = Array.from(clientMap, ([code, purchases]) => ({ code, purchases }));
  const products = Array.from(productMap.values());
  const purchaseCount = clients.reduce((total, client) => total + client.purchases.size, 0);
  return { source, clients, products, purchaseCount };
}

function intersectionSize(left: Set<string>, right: Set<string>) {
  let matches = 0;
  left.forEach((item) => { if (right.has(item)) matches += 1; });
  return matches;
}

export function recommendForClient(dataset: Dataset, clientCode: string, quantity: number) {
  const target = dataset.clients.find((client) => client.code === clientCode);
  if (!target) return { recommendations: [], closestClient: null, closestAffinity: 0 };
  const neighbors = dataset.clients
    .filter((client) => client.code !== clientCode)
    .map((client) => {
      const intersection = intersectionSize(target.purchases, client.purchases);
      const distance = target.purchases.size > 0 ? 1 - intersection / target.purchases.size : 1;
      return { client, intersection, distance };
    })
    .sort((left, right) => left.distance - right.distance || left.client.code.localeCompare(right.client.code));
  const scores = dataset.products
    .filter((product) => !target.purchases.has(product.code))
    .map((product) => {
      let score = 1;
      let supporters = 0;
      neighbors.forEach((neighbor) => {
        if (neighbor.intersection > 0 && neighbor.client.purchases.has(product.code)) {
          score *= neighbor.distance;
          supporters += 1;
        }
      });
      return { product, affinity: supporters > 0 ? 1 - score : 0, supporters };
    })
    .sort((left, right) => right.affinity - left.affinity || left.product.code.localeCompare(right.product.code))
    .slice(0, Math.max(1, quantity));
  const closest = neighbors[0];
  return {
    recommendations: scores,
    closestClient: closest?.intersection ? closest.client.code : null,
    closestAffinity: closest?.intersection ? 1 - closest.distance : 0,
  };
}
