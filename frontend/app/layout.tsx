import type { Metadata } from "next";
import { DM_Sans, Manrope } from "next/font/google";
import "./globals.css";

const bodyFont = DM_Sans({ variable: "--font-body", subsets: ["latin"] });
const displayFont = Manrope({ variable: "--font-display", subsets: ["latin"] });

export const metadata: Metadata = {
  title: "Recomenda | Sistema de recomendação",
  description: "Analise históricos de compra e gere recomendações de produtos por similaridade entre clientes.",
};

export default function RootLayout({ children }: Readonly<{ children: React.ReactNode }>) {
  return (
    <html lang="pt-BR">
      <body className={`${bodyFont.variable} ${displayFont.variable}`}>{children}</body>
    </html>
  );
}
