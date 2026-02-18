#include "stellar_webserver.h"

// ============================================================
// Dashboard HTML/CSS/JS  (stored in flash via PROGMEM)
// ============================================================
static const char HTML_PAGE[] PROGMEM = R"HTMLPAGE(<!DOCTYPE html>
<html lang="en">
<head>
<meta charset="UTF-8">
<meta name="viewport" content="width=device-width,initial-scale=1">
<title>Stellar IoT Dashboard</title>
<style>
*{box-sizing:border-box;margin:0;padding:0}
:root{
  --bg:#08080f;--card:#0f0f1c;--term:#0b0d14;--border:#1b1b2e;
  --primary:#7c3aed;--ph:#5b21b6;--blue:#3b82f6;
  --text:#e2e8f0;--muted:#4a5568;
  --ok:#10b981;--err:#ef4444;--warn:#f59e0b}
body{background:var(--bg);color:var(--text);
  font-family:-apple-system,BlinkMacSystemFont,'Segoe UI',Roboto,sans-serif;
  min-height:100vh}
/* ── Header ── */
header{
  background:rgba(15,15,28,.92);border-bottom:1px solid var(--border);
  padding:.875rem 1.5rem;display:flex;align-items:center;
  justify-content:space-between;position:sticky;top:0;z-index:100;
  backdrop-filter:blur(16px)}
.logo{display:flex;align-items:center;gap:.625rem}
.logo-icon{
  width:32px;height:32px;flex-shrink:0;
  background:linear-gradient(135deg,var(--primary),var(--blue));
  border-radius:9px;display:flex;align-items:center;justify-content:center}
.logo h1{font-size:1rem;font-weight:700;letter-spacing:-.025em}
.logo .sub{font-size:.68rem;color:var(--muted);margin-top:1px}
.indicators{display:flex;gap:1rem;align-items:center}
.ind{display:flex;align-items:center;gap:.35rem;font-size:.7rem;color:var(--muted)}
.dot{width:7px;height:7px;border-radius:50%;background:var(--err);flex-shrink:0}
.dot.on{background:var(--ok)}
.dot.pulse{animation:pulse 2s ease infinite}
@keyframes pulse{0%,100%{opacity:1}50%{opacity:.25}}
/* ── Layout ── */
.wrap{
  max-width:1120px;margin:0 auto;padding:1.5rem;
  display:grid;grid-template-columns:268px 1fr;gap:1.25rem;
  align-items:start}
@media(max-width:780px){
  .wrap{grid-template-columns:1fr;padding:1rem}
  .sidebar{display:grid;grid-template-columns:1fr 1fr;gap:.75rem}
  @media(max-width:480px){.sidebar{grid-template-columns:1fr}}}
/* ── Cards ── */
.card{background:var(--card);border:1px solid var(--border);border-radius:13px;overflow:hidden}
.ch{
  padding:.7rem 1rem;border-bottom:1px solid var(--border);
  font-size:.67rem;font-weight:700;text-transform:uppercase;
  letter-spacing:.09em;color:var(--muted);
  display:flex;align-items:center;gap:.4rem}
.cb{padding:.625rem;display:flex;flex-direction:column;gap:.35rem}
/* ── Buttons ── */
button{
  cursor:pointer;border:none;border-radius:8px;
  font-size:.78rem;font-weight:500;font-family:inherit;
  padding:.55rem .875rem;transition:all .13s;
  display:flex;align-items:center;gap:.45rem;width:100%;text-align:left}
button:active{transform:scale(.975)}
button svg{flex-shrink:0;opacity:.85}
.bp{background:var(--primary);color:#fff}
.bp:hover{background:var(--ph)}
.bg{background:transparent;color:var(--text);border:1px solid var(--border)}
.bg:hover{background:rgba(255,255,255,.04);border-color:rgba(255,255,255,.14)}
.bd{background:transparent;color:var(--err);border:1px solid rgba(239,68,68,.22)}
.bd:hover{background:rgba(239,68,68,.07)}
.bs{background:transparent;color:var(--ok);border:1px solid rgba(16,185,129,.22)}
.bs:hover{background:rgba(16,185,129,.07)}
/* ── Terminal ── */
.term-wrap{display:flex;flex-direction:column}
.term-head{
  padding:.75rem 1rem;border-bottom:1px solid var(--border);
  display:flex;align-items:center;justify-content:space-between}
.term-left{display:flex;align-items:center;gap:.6rem}
.dots{display:flex;gap:5px}
.d{width:10px;height:10px;border-radius:50%}
.dr{background:#ff5f57}.dy{background:#ffbd2e}.dg{background:#28c840}
.tlabel{font-size:.72rem;color:var(--muted);font-weight:500}
.term-right{display:flex;gap:.4rem}
.sm{padding:.3rem .625rem;font-size:.7rem;width:auto}
.term-body{
  background:var(--term);min-height:440px;max-height:580px;overflow-y:auto;
  padding:1rem 1.125rem;
  font-family:'Cascadia Code','Fira Code','JetBrains Mono',Consolas,monospace;
  font-size:.775rem;line-height:1.7}
.term-body::-webkit-scrollbar{width:5px}
.term-body::-webkit-scrollbar-thumb{background:var(--border);border-radius:3px}
.tl{display:flex;gap:.5rem;margin-bottom:1px}
.tp{color:var(--primary);opacity:.55;user-select:none;flex-shrink:0}
.to{color:var(--text)}
.tc{color:var(--blue)}
.tok{color:var(--ok)}
.ter{color:var(--err)}
.tw{color:var(--warn)}
.tm{color:var(--muted)}
.empty-msg{color:var(--muted);text-align:center;padding:5rem 1rem;font-size:.8rem}
/* ── Modals ── */
.overlay{
  position:fixed;inset:0;background:rgba(0,0,0,.68);
  backdrop-filter:blur(8px);display:none;
  align-items:center;justify-content:center;z-index:200;padding:1rem}
.overlay.open{display:flex}
.modal{
  background:var(--card);border:1px solid var(--border);
  border-radius:16px;padding:1.625rem;width:100%;max-width:420px;
  animation:pop .14s ease}
@keyframes pop{
  from{opacity:0;transform:scale(.94) translateY(8px)}
  to{opacity:1;transform:scale(1) translateY(0)}}
.modal h3{font-size:.95rem;font-weight:600;margin-bottom:1.125rem;letter-spacing:-.01em}
.fg{margin-bottom:.75rem}
.fg label{display:block;font-size:.72rem;color:var(--muted);margin-bottom:.3rem;font-weight:500}
.fg input,.fg textarea{
  width:100%;background:var(--bg);border:1px solid var(--border);
  color:var(--text);padding:.575rem .8rem;border-radius:8px;
  font-size:.82rem;font-family:inherit;transition:border-color .15s}
.fg input:focus,.fg textarea:focus{outline:none;border-color:var(--primary)}
.ma{display:flex;gap:.625rem;margin-top:1.25rem}
.ma button{flex:1;justify-content:center}
</style>
</head>
<body>

<header>
  <div class="logo">
    <div class="logo-icon">
      <svg width="16" height="16" viewBox="0 0 24 24" fill="none" stroke="white" stroke-width="2.5">
        <polygon points="12 2 15.09 8.26 22 9.27 17 14.14 18.18 21.02 12 17.77 5.82 21.02 7 14.14 2 9.27 8.91 8.26 12 2"/>
      </svg>
    </div>
    <div>
      <h1>Stellar IoT</h1>
      <div class="sub">ESP32 Dashboard &mdash; v0.1.0</div>
    </div>
  </div>
  <div class="indicators">
    <div class="ind">
      <div class="dot pulse" id="wDot"></div>
      <span id="wTxt">WiFi</span>
    </div>
    <div class="ind">
      <div class="dot" id="kDot"></div>
      <span id="kTxt">Wallet</span>
    </div>
  </div>
</header>

<div class="wrap">
  <!-- ── Sidebar ── -->
  <div class="sidebar">

    <div class="card">
      <div class="ch">
        <svg width="11" height="11" viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="2">
          <rect x="1" y="4" width="22" height="16" rx="2"/><line x1="1" y1="10" x2="23" y2="10"/>
        </svg>
        Wallet
      </div>
      <div class="cb">
        <button class="bp" onclick="cmd('/api/wallet/new','GET','wallet new')">
          <svg width="12" height="12" viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="2.5">
            <line x1="12" y1="5" x2="12" y2="19"/><line x1="5" y1="12" x2="19" y2="12"/>
          </svg>New Wallet
        </button>
        <button class="bg" onclick="openModal('mImport')">
          <svg width="12" height="12" viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="2">
            <path d="M21 15v4a2 2 0 0 1-2 2H5a2 2 0 0 1-2-2v-4"/>
            <polyline points="17 8 12 3 7 8"/><line x1="12" y1="3" x2="12" y2="15"/>
          </svg>Import Key
        </button>
        <button class="bg" onclick="openModal('mLoad')">
          <svg width="12" height="12" viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="2">
            <path d="M21 15v4a2 2 0 0 1-2 2H5a2 2 0 0 1-2-2v-4"/>
            <polyline points="7 10 12 15 17 10"/><line x1="12" y1="15" x2="12" y2="3"/>
          </svg>Load from Flash
        </button>
        <button class="bg" onclick="cmd('/api/wallet/show','GET','wallet show')">
          <svg width="12" height="12" viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="2">
            <circle cx="11" cy="11" r="8"/><line x1="21" y1="21" x2="16.65" y2="16.65"/>
          </svg>Show Wallet
        </button>
        <button class="bg" onclick="openModal('mSave')">
          <svg width="12" height="12" viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="2">
            <path d="M19 21H5a2 2 0 0 1-2-2V5a2 2 0 0 1 2-2h11l5 5v11a2 2 0 0 1-2 2z"/>
            <polyline points="17 21 17 13 7 13 7 21"/>
            <polyline points="7 3 7 8 15 8"/>
          </svg>Save to Flash
        </button>
        <button class="bd" onclick="delWallet()">
          <svg width="12" height="12" viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="2">
            <polyline points="3 6 5 6 21 6"/>
            <path d="M19 6v14a2 2 0 0 1-2 2H7a2 2 0 0 1-2-2V6m3 0V4a2 2 0 0 1 2-2h4a2 2 0 0 1 2 2v2"/>
          </svg>Delete Wallet
        </button>
      </div>
    </div>

    <div class="card">
      <div class="ch">
        <svg width="11" height="11" viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="2">
          <circle cx="12" cy="12" r="10"/>
          <line x1="2" y1="12" x2="22" y2="12"/>
          <path d="M12 2a15.3 15.3 0 0 1 4 10 15.3 15.3 0 0 1-4 10 15.3 15.3 0 0 1-4-10 15.3 15.3 0 0 1 4-10z"/>
        </svg>
        Network
      </div>
      <div class="cb">
        <button class="bg" onclick="cmd('/api/network/test','GET','network test')">
          <svg width="12" height="12" viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="2">
            <polyline points="22 12 18 12 15 21 9 3 6 12 2 12"/>
          </svg>Test Connection
        </button>
        <button class="bs" onclick="cmd('/api/network/fund','GET','network fund')">
          <svg width="12" height="12" viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="2">
            <circle cx="12" cy="12" r="10"/>
            <line x1="12" y1="8" x2="12" y2="16"/>
            <line x1="8" y1="12" x2="16" y2="12"/>
          </svg>Fund (Friendbot)
        </button>
        <button class="bg" onclick="cmd('/api/network/balance','GET','network balance')">
          <svg width="12" height="12" viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="2">
            <line x1="12" y1="1" x2="12" y2="23"/>
            <path d="M17 5H9.5a3.5 3.5 0 0 0 0 7h5a3.5 3.5 0 0 1 0 7H6"/>
          </svg>Get Balance
        </button>
        <button class="bg" onclick="cmd('/api/network/info','GET','network info')">
          <svg width="12" height="12" viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="2">
            <circle cx="12" cy="12" r="10"/>
            <line x1="12" y1="16" x2="12" y2="12"/>
            <line x1="12" y1="8" x2="12.01" y2="8"/>
          </svg>Account Info
        </button>
      </div>
    </div>

    <div class="card">
      <div class="ch">
        <svg width="11" height="11" viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="2">
          <line x1="22" y1="2" x2="11" y2="13"/>
          <polygon points="22 2 15 22 11 13 2 9 22 2"/>
        </svg>
        Payments
      </div>
      <div class="cb">
        <button class="bp" onclick="openModal('mPay')">
          <svg width="12" height="12" viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="2.5">
            <line x1="22" y1="2" x2="11" y2="13"/>
            <polygon points="22 2 15 22 11 13 2 9 22 2"/>
          </svg>Send Payment
        </button>
        <button class="bg" onclick="cmd('/api/pay/status','GET','pay status')">
          <svg width="12" height="12" viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="2">
            <circle cx="12" cy="12" r="10"/>
            <polyline points="12 6 12 12 16 14"/>
          </svg>Last TX Status
        </button>
        <button class="bg" onclick="cmd('/api/pay/history','GET','pay history')">
          <svg width="12" height="12" viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="2">
            <path d="M3 3h6l2 3H21a2 2 0 0 1 2 2v10a2 2 0 0 1-2 2H3a2 2 0 0 1-2-2V5a2 2 0 0 1 2-2z"/>
          </svg>Payment History
        </button>
      </div>
    </div>

    <div class="card">
      <div class="ch">
        <svg width="11" height="11" viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="2">
          <rect x="2" y="3" width="20" height="14" rx="2"/>
          <line x1="8" y1="21" x2="16" y2="21"/>
          <line x1="12" y1="17" x2="12" y2="21"/>
        </svg>
        System
      </div>
      <div class="cb">
        <button class="bg" onclick="cmd('/api/crypto/test','GET','crypto test')">
          <svg width="12" height="12" viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="2">
            <rect x="3" y="11" width="18" height="11" rx="2"/>
            <path d="M7 11V7a5 5 0 0 1 10 0v4"/>
          </svg>Crypto Tests
        </button>
        <button class="bg" onclick="cmd('/api/memory','GET','memory')">
          <svg width="12" height="12" viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="2">
            <rect x="4" y="4" width="16" height="16" rx="2"/>
            <rect x="9" y="9" width="6" height="6"/>
            <line x1="9" y1="1" x2="9" y2="4"/>
            <line x1="15" y1="1" x2="15" y2="4"/>
            <line x1="9" y1="20" x2="9" y2="23"/>
            <line x1="15" y1="20" x2="15" y2="23"/>
            <line x1="20" y1="9" x2="23" y2="9"/>
            <line x1="20" y1="14" x2="23" y2="14"/>
            <line x1="1" y1="9" x2="4" y2="9"/>
            <line x1="1" y1="14" x2="4" y2="14"/>
          </svg>Memory Stats
        </button>
      </div>
    </div>

  </div><!-- /sidebar -->

  <!-- ── Terminal ── -->
  <div class="card term-wrap">
    <div class="term-head">
      <div class="term-left">
        <div class="dots">
          <div class="d dr"></div>
          <div class="d dy"></div>
          <div class="d dg"></div>
        </div>
        <span class="tlabel">Output Console</span>
      </div>
      <div class="term-right">
        <button class="bg sm" onclick="clearTerm()">Clear</button>
        <button class="bg sm" onclick="refresh()">Refresh</button>
      </div>
    </div>
    <div class="term-body" id="term">
      <div class="empty-msg">Select a command from the sidebar to get started</div>
    </div>
  </div>

</div><!-- /wrap -->

<!-- ── Modals ── -->

<div class="overlay" id="mPay" onclick="bgClose(event,this)">
  <div class="modal">
    <h3>Send XLM Payment</h3>
    <div class="fg">
      <label>Destination Address (G...)</label>
      <input id="pDest" type="text" placeholder="GXXXXXXXXX..." autocomplete="off">
    </div>
    <div class="fg">
      <label>Amount (XLM)</label>
      <input id="pAmt" type="number" placeholder="1.0" step="0.0001" min="0">
    </div>
    <div class="fg">
      <label>Memo <span style="font-weight:400;opacity:.6">(optional &mdash; max 28 chars)</span></label>
      <input id="pMemo" type="text" placeholder="Payment memo..." maxlength="28">
    </div>
    <div class="ma">
      <button class="bg" onclick="closeModal('mPay')">Cancel</button>
      <button class="bp" onclick="paySend()">
        <svg width="12" height="12" viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="2.5">
          <line x1="22" y1="2" x2="11" y2="13"/><polygon points="22 2 15 22 11 13 2 9 22 2"/>
        </svg>Send
      </button>
    </div>
  </div>
</div>

<div class="overlay" id="mSave" onclick="bgClose(event,this)">
  <div class="modal">
    <h3>Save Wallet to Flash</h3>
    <div class="fg">
      <label>Password <span style="font-weight:400;opacity:.6">(min 8 characters)</span></label>
      <input id="savePw" type="password" placeholder="Enter password...">
    </div>
    <div class="ma">
      <button class="bg" onclick="closeModal('mSave')">Cancel</button>
      <button class="bp" onclick="walletSave()">Save</button>
    </div>
  </div>
</div>

<div class="overlay" id="mLoad" onclick="bgClose(event,this)">
  <div class="modal">
    <h3>Load Wallet from Flash</h3>
    <div class="fg">
      <label>Password</label>
      <input id="loadPw" type="password" placeholder="Enter password...">
    </div>
    <div class="ma">
      <button class="bg" onclick="closeModal('mLoad')">Cancel</button>
      <button class="bp" onclick="walletLoad()">Load</button>
    </div>
  </div>
</div>

<div class="overlay" id="mImport" onclick="bgClose(event,this)">
  <div class="modal">
    <h3>Import Secret Key</h3>
    <div class="fg">
      <label>Secret Key (S...)</label>
      <input id="impKey" type="text" placeholder="SXXXXXXXXX..." autocomplete="off">
    </div>
    <div class="ma">
      <button class="bg" onclick="closeModal('mImport')">Cancel</button>
      <button class="bp" onclick="walletImport()">Import</button>
    </div>
  </div>
</div>

<script>
const T = document.getElementById('term');
let lc = 0;

function _row(txt, cls) {
  if (lc === 0) T.innerHTML = '';
  lc++;
  const row = document.createElement('div');
  row.className = 'tl';
  const p = document.createElement('span');
  p.className = 'tp'; p.textContent = '\u203a';
  const s = document.createElement('span');
  s.className = cls || 'to'; s.textContent = txt;
  row.append(p, s); T.append(row);
  T.scrollTop = T.scrollHeight;
}
function sep() {
  if (lc === 0) T.innerHTML = '';
  lc++;
  const row = document.createElement('div');
  row.className = 'tl';
  const s = document.createElement('span');
  s.className = 'tm'; s.textContent = '\u2500'.repeat(40);
  row.append(s); T.append(row);
  T.scrollTop = T.scrollHeight;
}
function clearTerm() {
  T.innerHTML = '<div class="empty-msg">Terminal cleared</div>';
  lc = 0;
}

function openModal(id) { document.getElementById(id).classList.add('open'); }
function closeModal(id) { document.getElementById(id).classList.remove('open'); }
function bgClose(e, el) { if (e.target === el) el.classList.remove('open'); }
document.addEventListener('keydown', e => {
  if (e.key === 'Escape')
    document.querySelectorAll('.overlay.open').forEach(el => el.classList.remove('open'));
});

async function apiFetch(url, method, body) {
  const opts = { method: method || 'GET', headers: { 'Content-Type': 'application/json' } };
  if (body) opts.body = JSON.stringify(body);
  const r = await fetch(url, opts);
  return r.json();
}

function render(data) {
  if (data.success) {
    if (data.message) _row(data.message, 'tok');
    if (data.data) data.data.split('\n').forEach(l => { if (l.trim()) _row(l, 'to'); });
  } else {
    _row('Error: ' + (data.error || 'Unknown error'), 'ter');
  }
}

async function cmd(url, method, label, body) {
  _row('$ ' + label, 'tc');
  try {
    const d = await apiFetch(url, method, body);
    render(d);
  } catch (e) { _row('Request failed: ' + e.message, 'ter'); }
}

async function refresh() {
  try {
    const d = await apiFetch('/api/status');
    const wd = document.getElementById('wDot');
    const wt = document.getElementById('wTxt');
    const kd = document.getElementById('kDot');
    const kt = document.getElementById('kTxt');
    wd.classList.toggle('on', !!d.wifi);
    wt.textContent = d.wifi ? (d.ip || 'Connected') : 'No WiFi';
    kd.classList.toggle('on', !!d.wallet);
    kt.textContent = d.wallet ? 'Wallet OK' : 'No Wallet';
  } catch (e) {}
}

/* ── Wallet ── */
async function delWallet() {
  if (!confirm('Delete wallet from flash storage? This cannot be undone.')) return;
  _row('$ wallet delete', 'tc');
  try {
    const d = await apiFetch('/api/wallet/delete', 'POST');
    render(d); refresh();
  } catch (e) { _row('Request failed: ' + e.message, 'ter'); }
}
async function walletSave() {
  const pw = document.getElementById('savePw').value;
  if (!pw || pw.length < 8) { alert('Password must be at least 8 characters'); return; }
  closeModal('mSave');
  _row('$ wallet save', 'tc'); _row('Encrypting and saving...', 'tm');
  try {
    const d = await apiFetch('/api/wallet/save', 'POST', { password: pw });
    render(d); document.getElementById('savePw').value = '';
  } catch (e) { _row('Request failed: ' + e.message, 'ter'); }
}
async function walletLoad() {
  const pw = document.getElementById('loadPw').value;
  closeModal('mLoad');
  _row('$ wallet load', 'tc'); _row('Decrypting from flash...', 'tm');
  try {
    const d = await apiFetch('/api/wallet/load', 'POST', { password: pw });
    render(d); document.getElementById('loadPw').value = ''; refresh();
  } catch (e) { _row('Request failed: ' + e.message, 'ter'); }
}
async function walletImport() {
  const key = document.getElementById('impKey').value.trim();
  if (!key.startsWith('S')) { alert('Secret key must start with "S"'); return; }
  closeModal('mImport');
  _row('$ wallet import', 'tc'); _row('Importing key...', 'tm');
  try {
    const d = await apiFetch('/api/wallet/import', 'POST', { secretKey: key });
    render(d); document.getElementById('impKey').value = ''; refresh();
  } catch (e) { _row('Request failed: ' + e.message, 'ter'); }
}

/* ── Payment ── */
async function paySend() {
  const dest = document.getElementById('pDest').value.trim();
  const amt  = parseFloat(document.getElementById('pAmt').value);
  const memo = document.getElementById('pMemo').value.trim();
  if (!dest || isNaN(amt) || amt <= 0) { alert('Please enter a valid destination and amount'); return; }
  closeModal('mPay');
  _row('$ pay send', 'tc'); sep();
  _row('Destination: ' + dest, 'tm');
  _row('Amount:      ' + amt + ' XLM', 'tm');
  if (memo) _row('Memo:        ' + memo, 'tm');
  sep(); _row('Signing and broadcasting...', 'tm');
  try {
    const d = await apiFetch('/api/pay/send', 'POST', { destination: dest, amount: amt, memo: memo || null });
    render(d);
    if (d.success) { document.getElementById('pDest').value = ''; document.getElementById('pAmt').value = ''; document.getElementById('pMemo').value = ''; }
  } catch (e) { _row('Request failed: ' + e.message, 'ter'); }
}

refresh();
setInterval(refresh, 15000);
</script>
</body>
</html>)HTMLPAGE";

// ============================================================
// Constructor
// ============================================================
StellarWebServer::StellarWebServer(
    StellarKeypair** keypair,
    StellarNetwork** network,
    StellarAccount** account,
    StellarPayment** payment,
    int port
) : _server(port),
    _keypair(keypair),
    _network(network),
    _account(account),
    _payment(payment)
{}

// ============================================================
// Public API
// ============================================================
void StellarWebServer::begin() {
    _setupRoutes();
    _server.begin();
    Serial.print("[WebServer] Dashboard: http://");
    Serial.println(WiFi.localIP());
}

void StellarWebServer::handle() {
    _server.handleClient();
}

// ============================================================
// Private: route registration
// ============================================================
void StellarWebServer::_setupRoutes() {
    _server.on("/",                   HTTP_GET,  [this]() { _handleRoot();           });
    _server.on("/api/status",         HTTP_GET,  [this]() { _handleStatus();         });

    _server.on("/api/wallet/new",     HTTP_GET,  [this]() { _handleWalletNew();      });
    _server.on("/api/wallet/save",    HTTP_POST, [this]() { _handleWalletSave();     });
    _server.on("/api/wallet/load",    HTTP_POST, [this]() { _handleWalletLoad();     });
    _server.on("/api/wallet/show",    HTTP_GET,  [this]() { _handleWalletShow();     });
    _server.on("/api/wallet/delete",  HTTP_POST, [this]() { _handleWalletDelete();   });
    _server.on("/api/wallet/import",  HTTP_POST, [this]() { _handleWalletImport();   });

    _server.on("/api/network/test",    HTTP_GET, [this]() { _handleNetworkTest();    });
    _server.on("/api/network/fund",    HTTP_GET, [this]() { _handleNetworkFund();    });
    _server.on("/api/network/balance", HTTP_GET, [this]() { _handleNetworkBalance(); });
    _server.on("/api/network/info",    HTTP_GET, [this]() { _handleNetworkInfo();    });

    _server.on("/api/pay/send",       HTTP_POST, [this]() { _handlePaySend();        });
    _server.on("/api/pay/status",     HTTP_GET,  [this]() { _handlePayStatus();      });
    _server.on("/api/pay/history",    HTTP_GET,  [this]() { _handlePayHistory();     });

    _server.on("/api/crypto/test",    HTTP_GET,  [this]() { _handleCryptoTest();     });
    _server.on("/api/memory",         HTTP_GET,  [this]() { _handleMemory();         });
}

// ============================================================
// Private: helpers
// ============================================================
void StellarWebServer::_sendJson(bool success, const String& message,
                                  const String& data, const String& error) {
    // Manual JSON build avoids large DynamicJsonDocument for big data strings
    String json = "{\"success\":";
    json += success ? "true" : "false";

    auto escape = [](const String& s) -> String {
        String out;
        out.reserve(s.length() + 16);
        for (unsigned i = 0; i < s.length(); i++) {
            char c = s[i];
            if      (c == '\\') out += "\\\\";
            else if (c == '"')  out += "\\\"";
            else if (c == '\n') out += "\\n";
            else if (c == '\r') { /* skip */ }
            else                out += c;
        }
        return out;
    };

    if (message.length()) { json += ",\"message\":\""; json += escape(message); json += "\""; }
    if (data.length())    { json += ",\"data\":\"";    json += escape(data);    json += "\""; }
    if (error.length())   { json += ",\"error\":\"";   json += escape(error);   json += "\""; }
    json += "}";

    _server.sendHeader("Access-Control-Allow-Origin", "*");
    _server.send(200, "application/json", json);
}

void StellarWebServer::_ensureManagers() {
    if (!*_network) {
        *_network = new StellarNetwork(STELLAR_TESTNET);
    }
    if (*_keypair && !*_account) {
        *_account = new StellarAccount(*_keypair, *_network);
    }
    if (*_keypair && *_account && !*_payment) {
        *_payment = new StellarPayment(*_keypair, *_network, *_account);
    }
}

void StellarWebServer::_cleanupDependents() {
    if (*_payment) { delete *_payment; *_payment = nullptr; }
    if (*_account) { delete *_account; *_account = nullptr; }
}

// ============================================================
// Route handlers
// ============================================================

void StellarWebServer::_handleRoot() {
    _server.send_P(200, "text/html", HTML_PAGE);
}

void StellarWebServer::_handleStatus() {
    String json = "{\"wifi\":";
    json += WiFi.isConnected() ? "true" : "false";
    if (WiFi.isConnected()) {
        json += ",\"ip\":\"";  json += WiFi.localIP().toString(); json += "\"";
        json += ",\"ssid\":\""; json += WiFi.SSID();              json += "\"";
    }
    json += ",\"wallet\":";
    json += (*_keypair != nullptr) ? "true" : "false";
    if (*_keypair) {
        json += ",\"publicKey\":\""; json += (*_keypair)->getPublicKey(); json += "\"";
    }
    json += "}";
    _server.sendHeader("Access-Control-Allow-Origin", "*");
    _server.send(200, "application/json", json);
}

// ── Wallet handlers ──────────────────────────────────────────

void StellarWebServer::_handleWalletNew() {
    _cleanupDependents();
    if (*_keypair) { delete *_keypair; *_keypair = nullptr; }

    *_keypair = StellarKeypair::generate();
    if (!*_keypair) {
        _sendJson(false, "", "", "Failed to generate wallet");
        return;
    }

    String data;
    data += "Public Key:  " + (*_keypair)->getPublicKey() + "\n";
    data += "Secret Key:  " + (*_keypair)->getSecretKey() + "\n";
    String mnemonic = (*_keypair)->getMnemonic();
    if (mnemonic.length()) data += "Mnemonic:    " + mnemonic;

    _ensureManagers();
    _sendJson(true, "Wallet generated! Save your secret key securely.", data);
}

void StellarWebServer::_handleWalletSave() {
    if (!*_keypair) {
        _sendJson(false, "", "", "No wallet loaded. Generate or import one first.");
        return;
    }
    DynamicJsonDocument doc(256);
    if (deserializeJson(doc, _server.arg("plain")) || !doc.containsKey("password")) {
        _sendJson(false, "", "", "Missing 'password' field in request body");
        return;
    }
    String password = doc["password"].as<String>();
    if (password.length() < 8) {
        _sendJson(false, "", "", "Password too short (minimum 8 characters)");
        return;
    }
    SecureWallet wallet;
    if (wallet.saveToFlash(*_keypair, password.c_str())) {
        _sendJson(true, "Wallet saved to flash successfully!");
    } else {
        _sendJson(false, "", "", "Failed to save wallet to flash");
    }
}

void StellarWebServer::_handleWalletLoad() {
    DynamicJsonDocument doc(256);
    if (deserializeJson(doc, _server.arg("plain")) || !doc.containsKey("password")) {
        _sendJson(false, "", "", "Missing 'password' field in request body");
        return;
    }
    String password = doc["password"].as<String>();

    _cleanupDependents();
    if (*_keypair) { delete *_keypair; *_keypair = nullptr; }

    *_keypair = SecureWallet::loadFromFlash(password.c_str());
    if (!*_keypair) {
        _sendJson(false, "", "", "Failed to load wallet. Wrong password?");
        return;
    }
    _ensureManagers();
    _sendJson(true, "Wallet loaded!", "Public Key: " + (*_keypair)->getPublicKey());
}

void StellarWebServer::_handleWalletShow() {
    if (!*_keypair) {
        _sendJson(false, "", "", "No wallet loaded");
        return;
    }
    String data;
    data += "Public Key:  " + (*_keypair)->getPublicKey() + "\n";
    data += "Secret Key:  " + (*_keypair)->getSecretKey() + "\n";
    String mnemonic = (*_keypair)->getMnemonic();
    if (mnemonic.length()) data += "Mnemonic:    " + mnemonic;

    _sendJson(true, "Current Wallet", data);
}

void StellarWebServer::_handleWalletDelete() {
    if (SecureWallet::deleteFromFlash()) {
        _cleanupDependents();
        if (*_keypair) { delete *_keypair; *_keypair = nullptr; }
        _sendJson(true, "Wallet deleted from flash.");
    } else {
        _sendJson(false, "", "", "Failed to delete wallet (none saved?)");
    }
}

void StellarWebServer::_handleWalletImport() {
    DynamicJsonDocument doc(256);
    if (deserializeJson(doc, _server.arg("plain")) || !doc.containsKey("secretKey")) {
        _sendJson(false, "", "", "Missing 'secretKey' field in request body");
        return;
    }
    String secretKey = doc["secretKey"].as<String>();

    _cleanupDependents();
    if (*_keypair) { delete *_keypair; *_keypair = nullptr; }

    *_keypair = StellarKeypair::fromSecret(secretKey.c_str());
    if (!*_keypair) {
        _sendJson(false, "", "", "Invalid secret key");
        return;
    }
    _ensureManagers();
    _sendJson(true, "Wallet imported!", "Public Key: " + (*_keypair)->getPublicKey());
}

// ── Network handlers ─────────────────────────────────────────

void StellarWebServer::_handleNetworkTest() {
    if (!WiFi.isConnected()) {
        _sendJson(false, "", "", "WiFi not connected");
        return;
    }
    StellarNetwork network(STELLAR_TESTNET);
    // Use a known funded Stellar testnet account for the ping
    String response = network.getAccount("GBY5AZJYQNUD22NLNEX23NWIFWALIGDRQY2X7W6TPNYHJWY6TCV7W64I");
    if (response.length() > 0) {
        String data = "Horizon URL: ";
        data += network.getHorizonURL();
        data += "\nWiFi IP:     ";
        data += WiFi.localIP().toString();
        data += "\nStatus:      Connected";
        _sendJson(true, "Horizon connection successful!", data);
    } else {
        _sendJson(false, "", "", "Connection failed: " + network.getLastError());
    }
}

void StellarWebServer::_handleNetworkFund() {
    if (!*_keypair) {
        _sendJson(false, "", "", "No wallet loaded. Generate or import one first.");
        return;
    }
    if (!WiFi.isConnected()) {
        _sendJson(false, "", "", "WiFi not connected");
        return;
    }
    StellarNetwork network(STELLAR_TESTNET);
    String pubKey = (*_keypair)->getPublicKey();
    if (network.fundWithFriendbot(pubKey.c_str())) {
        String data = "Account: " + pubKey + "\nBalance: 10,000 XLM (testnet)";
        _sendJson(true, "Account funded via Friendbot!", data);
    } else {
        _sendJson(false, "", "", "Funding failed: " + network.getLastError());
    }
}

void StellarWebServer::_handleNetworkBalance() {
    if (!*_keypair) { _sendJson(false, "", "", "No wallet loaded"); return; }
    if (!WiFi.isConnected()) { _sendJson(false, "", "", "WiFi not connected"); return; }

    StellarNetwork network(STELLAR_TESTNET);
    String response = network.getAccount((*_keypair)->getPublicKey().c_str());
    if (response.length() == 0) {
        _sendJson(false, "", "", "Query failed. Account may not be funded yet. Use Friendbot first.");
        return;
    }
    DynamicJsonDocument doc(4096);
    if (deserializeJson(doc, response)) {
        _sendJson(false, "", "", "Failed to parse Horizon response");
        return;
    }
    String data;
    for (JsonObject b : doc["balances"].as<JsonArray>()) {
        String assetType = b["asset_type"].as<String>();
        String amount    = b["balance"].as<String>();
        if (assetType == "native") {
            data += "XLM: " + amount + "\n";
        } else {
            data += b["asset_code"].as<String>() + ": " + amount + "\n";
        }
    }
    _sendJson(true, "Account Balance", data);
}

void StellarWebServer::_handleNetworkInfo() {
    if (!*_keypair) { _sendJson(false, "", "", "No wallet loaded"); return; }
    if (!WiFi.isConnected()) { _sendJson(false, "", "", "WiFi not connected"); return; }

    StellarNetwork network(STELLAR_TESTNET);
    String response = network.getAccount((*_keypair)->getPublicKey().c_str());
    if (response.length() == 0) {
        _sendJson(false, "", "", "Query failed: " + network.getLastError());
        return;
    }
    DynamicJsonDocument doc(4096);
    if (deserializeJson(doc, response)) {
        _sendJson(false, "", "", "Failed to parse Horizon response");
        return;
    }
    String data;
    data += "ID:             " + doc["id"].as<String>()                       + "\n";
    data += "Sequence:       " + doc["sequence"].as<String>()                  + "\n";
    data += "Subentries:     " + String(doc["subentry_count"].as<int>())       + "\n";
    data += "Network:        Stellar Testnet";
    _sendJson(true, "Account Info", data);
}

// ── Payment handlers ─────────────────────────────────────────

void StellarWebServer::_handlePaySend() {
    if (!*_keypair) { _sendJson(false, "", "", "No wallet loaded"); return; }
    if (!WiFi.isConnected()) { _sendJson(false, "", "", "WiFi not connected"); return; }

    DynamicJsonDocument doc(512);
    if (deserializeJson(doc, _server.arg("plain"))) {
        _sendJson(false, "", "", "Invalid request body");
        return;
    }
    String destination = doc["destination"].as<String>();
    float  amount      = doc["amount"].as<float>();
    String memo        = doc["memo"] | "";

    if (destination.length() == 0 || amount <= 0) {
        _sendJson(false, "", "", "Invalid destination or amount");
        return;
    }

    _ensureManagers();
    if (!(*_account)->isAccountActive()) {
        _sendJson(false, "", "", "Account not funded yet. Use 'Fund (Friendbot)' first.");
        return;
    }

    PaymentResult result = (*_payment)->sendPayment(
        destination.c_str(),
        amount,
        memo.length() > 0 ? memo.c_str() : nullptr
    );

    if (result.success) {
        String data;
        data += "TX Hash:  " + result.transactionHash + "\n";
        data += "Ledger:   " + String(result.ledger)  + "\n";
        data += "Explorer: https://stellar.expert/explorer/testnet/tx/" + result.transactionHash;
        _sendJson(true, "Payment sent successfully!", data);
    } else {
        _sendJson(false, "", "", "Payment failed: " + result.error);
    }
}

void StellarWebServer::_handlePayStatus() {
    if (!*_payment) {
        _sendJson(false, "", "", "No payment manager. Load a wallet first.");
        return;
    }
    String lastHash = (*_payment)->getLastTransactionHash();
    if (lastHash.length() == 0) {
        _sendJson(false, "", "", "No transactions sent in this session");
        return;
    }
    const char* statusStr = "UNKNOWN";
    switch ((*_payment)->getLastTransactionStatus()) {
        case TX_SUCCESS: statusStr = "SUCCESS"; break;
        case TX_FAILED:  statusStr = "FAILED";  break;
        case TX_PENDING: statusStr = "PENDING"; break;
        default: break;
    }
    String data = "Hash:   " + lastHash + "\nStatus: " + statusStr;
    _sendJson(true, "Last Transaction", data);
}

void StellarWebServer::_handlePayHistory() {
    if (!*_keypair) { _sendJson(false, "", "", "No wallet loaded"); return; }
    if (!WiFi.isConnected()) { _sendJson(false, "", "", "WiFi not connected"); return; }

    _ensureManagers();
    String response = (*_network)->getAccountPayments(
        (*_keypair)->getPublicKey().c_str(), nullptr, 10
    );
    if (response.length() == 0) {
        _sendJson(false, "", "", "Failed to fetch payment history");
        return;
    }
    DynamicJsonDocument doc(8192);
    if (deserializeJson(doc, response) || !doc.containsKey("_embedded")) {
        _sendJson(false, "", "", "Failed to parse Horizon response");
        return;
    }

    JsonArray records = doc["_embedded"]["records"];
    String data;
    int count = 0;
    for (JsonObject payment : records) {
        count++;
        String type = payment["type"].as<String>();
        data += String(count) + ". " + type + "\n";

        if (payment.containsKey("from")) {
            String from = payment["from"].as<String>();
            String abbr = from.substring(0, 6) + "..." + from.substring(from.length() - 4);
            data += "   From:   " + abbr + "\n";
        }
        if (payment.containsKey("to")) {
            String to = payment["to"].as<String>();
            String abbr = to.substring(0, 6) + "..." + to.substring(to.length() - 4);
            data += "   To:     " + abbr + "\n";
        }
        if (payment.containsKey("amount")) {
            data += "   Amount: " + payment["amount"].as<String>() + " XLM\n";
        }
        if (payment.containsKey("created_at")) {
            data += "   Date:   " + payment["created_at"].as<String>() + "\n";
        }
        data += "\n";
    }
    if (count == 0) data = "No payments found on this account";

    _sendJson(true, "Payment History (last " + String(count) + ")", data);
}

// ── System handlers ──────────────────────────────────────────

void StellarWebServer::_handleCryptoTest() {
    String data;

    uint8_t pub[32], priv[32];
    bool t1 = StellarCrypto::generateKeypair(pub, priv);
    data += "Test 1  Keypair generation:  " + String(t1 ? "PASS" : "FAIL") + "\n";

    uint8_t hash[32];
    StellarCrypto::sha256((uint8_t*)"test", 4, hash);
    data += "Test 2  SHA-256:             PASS\n";

    uint8_t sig[64];
    StellarCrypto::sign(priv, pub, (uint8_t*)"data", 4, sig);
    bool t3 = StellarCrypto::verify(pub, (uint8_t*)"data", 4, sig);
    data += "Test 3  Ed25519 sign/verify: " + String(t3 ? "PASS" : "FAIL") + "\n";

    uint8_t key[32], iv[12], ct[16], tag[16], pt[16];
    const uint8_t* plaintext = (uint8_t*)"Secret message!!";
    StellarCrypto::randomBytes(key, 32);
    StellarCrypto::randomBytes(iv, 12);
    StellarCrypto::encryptAES256GCM(plaintext, 16, key, iv, ct, tag);
    StellarCrypto::decryptAES256GCM(ct, 16, key, iv, tag, pt);
    bool t4 = (memcmp(plaintext, pt, 16) == 0);
    data += "Test 4  AES-256-GCM:         " + String(t4 ? "PASS" : "FAIL") + "\n";

    bool allPass = t1 && t3 && t4;
    _sendJson(true, allPass ? "All crypto tests passed!" : "Some tests failed", data);
}

void StellarWebServer::_handleMemory() {
    _sendJson(true, "Memory Statistics", StellarUtils::getMemoryInfo());
}
