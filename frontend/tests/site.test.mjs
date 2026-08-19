import assert from "node:assert/strict";
import test from "node:test";

import {
  DEMO_ROWS,
  buildDataset,
  parsePurchaseCsv,
  recommendForClient,
} from "../lib/recommender.ts";

async function render() {
  const workerUrl = new URL("../dist/server/index.js", import.meta.url);
  workerUrl.searchParams.set("test", `${process.pid}-${Date.now()}`);
  const { default: worker } = await import(workerUrl.href);
  return worker.fetch(
    new Request("http://localhost/", { headers: { accept: "text/html" } }),
    { ASSETS: { fetch: async () => new Response("Not found", { status: 404 }) } },
    { waitUntil() {}, passThroughOnException() {} },
  );
}

test("renderiza a página principal com conteúdo do produto", async () => {
  const response = await render();
  assert.equal(response.status, 200);
  const html = await response.text();
  assert.match(html, /<title>Recomenda \| Sistema de recomendação<\/title>/i);
  assert.match(html, /Encontre o próximo produto certo para cada cliente/);
  assert.match(html, /Gerar recomendações/);
  assert.doesNotMatch(html, /Your site is taking shape|SkeletonPreview|codex-preview/);
});

test("interpreta CSV com vírgula, aspas e compras duplicadas", () => {
  const rows = parsePurchaseCsv([
    "DATA,COD_CLIENTE,COD_PRODUTO,NOME_PRODUTO",
    '20260101,C1,P1,"Produto, Um"',
    '20260102,C1,P1,"Produto, Um"',
    "20260103,C2,P2,Produto Dois",
  ].join("\n"));
  const dataset = buildDataset(rows, "teste.csv");
  assert.equal(rows[0].productName, "Produto, Um");
  assert.equal(dataset.clients.length, 2);
  assert.equal(dataset.purchaseCount, 2);
});

test("recomenda somente produtos ainda não comprados", () => {
  const dataset = buildDataset(DEMO_ROWS, "demo");
  const result = recommendForClient(dataset, "CL-1042", 4);
  const purchased = dataset.clients.find((client) => client.code === "CL-1042").purchases;
  assert.equal(result.recommendations.length, 4);
  assert.ok(result.recommendations.every((item) => !purchased.has(item.product.code)));
  assert.equal(result.closestClient, "CL-1158");
  assert.ok(result.recommendations[0].affinity >= result.recommendations[1].affinity);
});
