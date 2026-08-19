# Frontend Recomenda

Interface web do sistema de recomendação. O processamento acontece no navegador:
o usuário pode carregar um CSV, escolher um cliente, gerar recomendações e baixar
o resultado.

## Desenvolvimento

Requer Node.js 22 ou mais recente.

```bash
npm install
npm run dev
```

Abra `http://localhost:3000`.

## Validação

```bash
npm test
npm run lint
```

O CSV deve usar vírgula ou ponto e vírgula e conter, nesta ordem, data, código do
cliente, código do produto e nome do produto. O parser aceita nomes entre aspas e
remove compras repetidas do mesmo produto pelo mesmo cliente.
