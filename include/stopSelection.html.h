/*
 * File: /include/stopSelection.html.h
 * Project: ESP32 TramTracker
 * Created Date: 17-10-2026, 5:08:23 pm
 * Author: Theo Gimenez
 * -----
 * Last Modified: Sunday, October 18th 2026, 6:03:56 pm
 * Modified By: Theo Gimenez
 * -----
 */

#ifndef INCLUDE_STOP_SELECTION_HTML_H_
#define INCLUDE_STOP_SELECTION_HTML_H_

namespace web {
/*
inline constexpr const char* k_stop_selection_html PROGMEM = R"rawliteral(
<!DOCTYPE html>
<html lang="fr">
<meta charset="utf-8">
<meta name="viewport" content="width=device-width,initial-scale=1">
<style>
body{font:16px system-ui;max-width:320px;margin:40px auto;padding:0 16px;background:#f5f5f5;color:#222}
select{width:100%%; padding:12px;margin:8px 0 20px;border:1px solid #ccc;border-radius:8px;background:white}
#s{font-size:14px} .ok{color:#198754}
</style>

<h3>Choix du trajet</h3>
<label for="r">Ligne</label>
<select id="r" onchange="loadStops()">
<option value="">Choisir une ligne</option>
%ROUTES%
</select>

<label for="s">Arrêt</label>
<select id="s" disabled onchange="save()">
<option value="">Choisir un arrêt</option>
</select>
<p id="msg"></p>

<script>
const r=document.getElementById('r'),s=document.getElementById('s'),m=document.getElementById('msg');
async function loadStops(){
 s.disabled=true;s.innerHTML='<option>Chargement...</option>';m.textContent='';
 if(!r.value){s.innerHTML='<option value="" disabled>Choisir un arrêt</option>';return}
 try{
  let a=await fetch('/stops?route='+encodeURIComponent(r.value));
  if(!a.ok)throw 0;
  let data=await a.json();
  s.innerHTML='<option value="">Choisir un arrêt</option>';
  data.forEach(v=>{let o=document.createElement('option');o.value=v;o.textContent=v;s.append(o)});
  s.disabled=!data.length;
 }catch(e){s.innerHTML='<option value="">Erreur de chargement</option>'}
}
async function save(){
 if(!s.value)return;
 m.textContent='Enregistrement...';m.className='';
 try{
  let a=await fetch('/select?route='+encodeURIComponent(r.value)+'&stop='+encodeURIComponent(s.value), {method:'POST'});
  if(a.status!==200)throw 0;
  m.textContent='✓ Sélection enregistrée';m.className='ok';
 }catch(e){m.textContent='Échec de l’enregistrement'}
}
</script>
)rawliteral";
*/
inline constexpr const char* k_stop_selection_html PROGMEM = R"rawliteral(
<!DOCTYPE html>
<html lang="fr">
<meta charset="utf-8">
<meta name="viewport" content="width=device-width,initial-scale=1">
<style>
*{box-sizing:border-box}
body{font:15px system-ui,sans-serif;max-width:360px;margin:0 auto;padding:32px 18px;background:#f3f5f9;color:#202938}
main{background:white;padding:24px 20px;border:1px solid #e5e7eb;border-radius:16px;box-shadow:0 8px 28px #1725540b}
h3{font-size:21px;margin:0 0 5px;letter-spacing:-.5px}
p.sub{font-size:13px;color:#7b8494;margin:0 0 25px}
label{display:block;font-size:13px;font-weight:600;margin:18px 0 7px}
select{display:block;width:100%%;padding:12px;border:1px solid #d9dee8;border-radius:9px;background:#fff;color:#202938;font:inherit;outline:none}
select:focus{border-color:#6366f1;box-shadow:0 0 0 3px #6366f120}
select:disabled{background:#f5f6f8;color:#9ca3af}
#msg{font-size:13px;margin:18px 0 0}
.ok{color:#16834a}
.err{color:#c0392b}
.wait{color:#717b8c}
</style>
<main>
<h3>Choix du trajet</h3>
<p class="sub">Sélectionnez votre ligne et votre arrêt.</p>
<label for="r">Ligne de transport</label>
<select id="r" onchange="loadStops()">
<option value="">Choisir une ligne</option>
%ROUTES%
</select>
<label for="s">Arrêt</label>
<select id="s" disabled onchange="save()">
<option value="">Choisir une ligne d'abord</option>
</select>
<p id="msg" aria-live="polite"></p>
</main>
<script>
const r=document.getElementById('r'),s=document.getElementById('s'),m=document.getElementById('msg');
async function loadStops(){
s.disabled=true;s.innerHTML='<option value="">Chargement...</option>';m.textContent='';m.className='';
if(!r.value){s.innerHTML='<option value="">Choisir une ligne d’abord</option>';return}
try{
let a=await fetch('/stops?route='+encodeURIComponent(r.value));
if(!a.ok)throw 0;
let d=await a.json();
s.innerHTML='<option value="">Choisir un arrêt</option>';
d.forEach(v=>{let o=document.createElement('option');o.value=v;o.textContent=v;s.append(o)});
s.disabled=!d.length;
if(!d.length)m.textContent='Aucun arrêt disponible';
}catch(e){s.innerHTML='<option value="">Erreur de chargement</option>';m.textContent='Impossible de charger les arrêts';m.className='err'}
}
async function save(){
if(!s.value)return;
m.textContent='Enregistrement...';m.className='wait';s.disabled=true;
try{
let a=await fetch('/select?route='+encodeURIComponent(r.value)+'&stop='+encodeURIComponent(s.value),{method:'POST'});
if(a.status!==200)throw 0;
m.textContent='✓ Sélection enregistrée';m.className='ok';
}catch(e){m.textContent='✕ Échec de l’enregistrement';m.className='err'}
finally{s.disabled=false}
}
</script>
</html>
)rawliteral";

}// namespace web

#endif // INCLUDE_STOP_SELECTION_HTML_H_