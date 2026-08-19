"use client";

import { ChangeEvent, useMemo, useRef, useState } from "react";
import {
  DEMO_ROWS,
  buildDataset,
  parsePurchaseCsv,
  recommendForClient,
  type Dataset,
  type Recommendation,
} from "../lib/recommender";

const demoDataset = buildDataset(DEMO_ROWS, "Base de demonstração");

function formatPercent(value: number) {
  return `${Math.round(value * 100)}%`;
}

export default function Home() {
  const fileInputRef = useRef<HTMLInputElement>(null);
  const [dataset, setDataset] = useState<Dataset>(demoDataset);
  const [clientCode, setClientCode] = useState(demoDataset.clients[0]?.code ?? "");
  const [quantity, setQuantity] = useState(4);
  const [recommendations, setRecommendations] = useState<Recommendation[]>([]);
  const [closestClient, setClosestClient] = useState<string | null>(null);
  const [closestAffinity, setClosestAffinity] = useState(0);
  const [hasRun, setHasRun] = useState(false);
  const [notice, setNotice] = useState("Pronto para analisar a base de demonstração.");
  const [isLoading, setIsLoading] = useState(false);

  const selectedClient = useMemo(
    () => dataset.clients.find((client) => client.code === clientCode),
    [clientCode, dataset.clients],
  );

  function runRecommendation() {
    const result = recommendForClient(dataset, clientCode, quantity);
    setRecommendations(result.recommendations);
    setClosestClient(result.closestClient);
    setClosestAffinity(result.closestAffinity);
    setHasRun(true);
    setNotice(
      result.recommendations.length > 0
        ? `${result.recommendations.length} recomendações calculadas.`
        : "Não há produtos novos para recomendar a este cliente.",
    );
  }

  async function handleFile(event: ChangeEvent<HTMLInputElement>) {
    const file = event.target.files?.[0];
    if (!file) return;

    setIsLoading(true);
    setNotice(`Lendo ${file.name}...`);

    try {
      const text = await file.text();
      const rows = parsePurchaseCsv(text);
      const nextDataset = buildDataset(rows, file.name);

      if (nextDataset.clients.length < 2 || nextDataset.products.length === 0) {
        throw new Error("A base precisa ter pelo menos dois clientes e um produto.");
      }

      setDataset(nextDataset);
      setClientCode(nextDataset.clients[0].code);
      setRecommendations([]);
      setHasRun(false);
      setNotice(`${file.name} carregado com sucesso.`);
    } catch (error) {
      setNotice(error instanceof Error ? error.message : "Não foi possível ler o arquivo.");
    } finally {
      setIsLoading(false);
      event.target.value = "";
    }
  }

  function restoreDemo() {
    setDataset(demoDataset);
    setClientCode(demoDataset.clients[0]?.code ?? "");
    setRecommendations([]);
    setHasRun(false);
    setNotice("Base de demonstração restaurada.");
  }

  function downloadRecommendations() {
    if (recommendations.length === 0) return;

    const lines = [
      "POSICAO,COD_CLIENTE,COD_PRODUTO,NOME_PRODUTO,AFINIDADE",
      ...recommendations.map((item, index) =>
        [
          index + 1,
          clientCode,
          item.product.code,
          `"${item.product.name.replaceAll('"', '""')}"`,
          Math.round(item.affinity * 100),
        ].join(","),
      ),
    ];
    const blob = new Blob([lines.join("\n")], { type: "text/csv;charset=utf-8" });
    const link = document.createElement("a");
    link.href = URL.createObjectURL(blob);
    link.download = `recomendacoes-${clientCode}.csv`;
    link.click();
    URL.revokeObjectURL(link.href);
  }

  return (
    <main>
      <header className="topbar">
        <a className="brand" href="#inicio" aria-label="Recomenda, início">
          <span className="brand-mark" aria-hidden="true">R</span>
          <span>recomenda</span>
        </a>
        <span className="engine-badge"><i /> Motor de recomendação ativo</span>
      </header>

      <section className="hero" id="inicio">
        <div>
          <p className="eyebrow">Sistema de recomendação</p>
          <h1>Encontre o próximo produto certo para cada cliente.</h1>
          <p className="hero-copy">
            Carregue o histórico de compras, escolha um cliente e receba uma lista
            calculada pela similaridade entre perfis.
          </p>
        </div>
        <div className="hero-metric" aria-label="Resumo da base atual">
          <span>Base atual</span>
          <strong>{dataset.source}</strong>
          <small>{dataset.purchaseCount.toLocaleString("pt-BR")} compras válidas</small>
        </div>
      </section>

      <section className="stats" aria-label="Estatísticas da base">
        <article><span>Clientes</span><strong>{dataset.clients.length.toLocaleString("pt-BR")}</strong></article>
        <article><span>Produtos</span><strong>{dataset.products.length.toLocaleString("pt-BR")}</strong></article>
        <article><span>Compras únicas</span><strong>{dataset.purchaseCount.toLocaleString("pt-BR")}</strong></article>
      </section>

      <section className="workspace">
        <aside className="control-panel">
          <div className="section-heading">
            <span className="step">01</span>
            <div><h2>Prepare a análise</h2><p>Use a amostra ou envie seu CSV.</p></div>
          </div>

          <div className="upload-box">
            <input
              ref={fileInputRef}
              type="file"
              accept=".csv,text/csv"
              onChange={handleFile}
              aria-label="Selecionar arquivo CSV"
            />
            <span className="upload-icon" aria-hidden="true">↑</span>
            <strong>Carregue seu histórico</strong>
            <small>CSV com cliente, produto e nome nas quatro primeiras colunas</small>
            <button type="button" className="secondary-button" onClick={() => fileInputRef.current?.click()} disabled={isLoading}>
              {isLoading ? "Lendo arquivo..." : "Escolher arquivo"}
            </button>
            {dataset.source !== demoDataset.source && (
              <button type="button" className="text-button" onClick={restoreDemo}>Voltar à demonstração</button>
            )}
          </div>

          <label>
            Cliente
            <select value={clientCode} onChange={(event) => { setClientCode(event.target.value); setHasRun(false); }}>
              {dataset.clients.map((client) => (
                <option value={client.code} key={client.code}>{client.code}</option>
              ))}
            </select>
          </label>

          <label>
            Quantidade de recomendações
            <input
              type="number"
              min="1"
              max="12"
              value={quantity}
              onChange={(event) => setQuantity(Math.min(12, Math.max(1, Number(event.target.value) || 1)))}
            />
          </label>

          <div className="method-note">
            <span>Modelo</span>
            <strong>Similaridade direcionada</strong>
            <small>Mesma regra de distância usada pelo núcleo do projeto.</small>
          </div>

          <button type="button" className="primary-button" onClick={runRecommendation}>
            Gerar recomendações <span aria-hidden="true">→</span>
          </button>
          <p className="status" role="status">{notice}</p>
        </aside>

        <section className="results-panel" aria-live="polite">
          <div className="results-header">
            <div className="section-heading">
              <span className="step">02</span>
              <div><h2>Recomendações</h2><p>Produtos ordenados pela afinidade calculada.</p></div>
            </div>
            {recommendations.length > 0 && (
              <button type="button" className="download-button" onClick={downloadRecommendations}>Baixar CSV</button>
            )}
          </div>

          {!hasRun ? (
            <div className="empty-state">
              <span className="empty-orbit"><i /></span>
              <h3>Sua lista aparecerá aqui</h3>
              <p>Escolha um cliente e inicie o cálculo para comparar os perfis da base.</p>
            </div>
          ) : (
            <>
              <div className="client-summary">
                <div>
                  <span>Cliente analisado</span>
                  <strong>{clientCode}</strong>
                  <small>{selectedClient?.purchases.size ?? 0} produtos no histórico</small>
                </div>
                <div>
                  <span>Perfil mais próximo</span>
                  <strong>{closestClient ?? "Sem correspondência"}</strong>
                  <small>{closestClient ? `${formatPercent(closestAffinity)} de afinidade` : "Nenhuma compra em comum"}</small>
                </div>
              </div>

              {recommendations.length > 0 ? (
                <ol className="recommendation-list">
                  {recommendations.map((item, index) => (
                    <li key={item.product.code}>
                      <span className="rank">{String(index + 1).padStart(2, "0")}</span>
                      <div className="product-copy">
                        <strong>{item.product.name}</strong>
                        <small>Código {item.product.code} · {item.supporters} {item.supporters === 1 ? "perfil relacionado" : "perfis relacionados"}</small>
                      </div>
                      <div className="affinity">
                        <span>{formatPercent(item.affinity)}</span>
                        <i><b style={{ width: formatPercent(item.affinity) }} /></i>
                      </div>
                    </li>
                  ))}
                </ol>
              ) : (
                <div className="empty-state compact">
                  <h3>Cliente já cobriu todo o catálogo</h3>
                  <p>Não há um produto novo disponível nesta base.</p>
                </div>
              )}
            </>
          )}
        </section>
      </section>

      <footer>
        <span>recomenda</span>
        <p>Processamento local. O arquivo enviado permanece no seu navegador.</p>
      </footer>
    </main>
  );
}
