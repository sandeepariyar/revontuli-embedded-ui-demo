const $=s=>document.querySelector(s), T=s=>`<span data-r>${s}</span>`;

const tabs=['Home','Comfort','Atmosphere','Media','Cabin'];

const seqNames=['home-heat-fx-1','home-heat-fx-2','home-heat-fx-3','climate-cool-face','climate-cool-body','climate-cool-feet','climate-warm-face','climate-warm-body','climate-warm-feet','window','shade','roof','recline','heat','cool','massage','airflow-face','airflow-body','airflow-feet','airflow-face-vent','airflow-body-vent','airflow-feet-vent','light-relax','home-heat','light-aurora','light-lounge','light-club','light-cinema'];

const initialSeat=()=>({temp:21.5,window:0,shade:0,heat:2,cool:0,recline:0,massage:false,air:'Face',fan:3});

let windowMotion=null;
const S={tab:'Home',comfortMode:'Climate',seat:'Rear left',seats:{'Rear left':initialSeat(),'Rear right':initialSeat()},scene:'aurora',brightness:85,light:true,beat:false,speed:'Flow',roof:0,auto:true,ac:true,reading:false,privacy:false,fresh:true,zones:['Doors','Roof','Footwell'],playing:false,volume:45,progress:84,track:0,source:'Spotify',output:'Cabin speakers',focus:'All seats',lastVolume:45,seatPreview:'recline',repeat:false,shuffle:false};

let theme='night';try{theme=localStorage.getItem('revontuli-theme')==='day'?'day':'night'}catch{}
const seat=()=>S.seats[S.seat], tracks=['Chromatic Waves','Northern Lights','Still Water'];

let tick=0,toastTimer,windowTimer,galleryTimer=null,lastPaint='',imageCache=new Map();

const paths={Home:'M3 11 L12 3 21 11 V21 H15 V14 H9 V21 H3 Z',Comfort:'M7 3 H11 L9 14 H20 V18 H5 Z M5 21 H20',Atmosphere:'M12 2 Q14 10 22 12 Q14 14 12 22 Q10 14 2 12 Q10 10 12 2 Z',Media:'M4 3 H20 V21 H4 Z M10 8 L16 12 10 16 Z',Cabin:'M3 10 L6 4 H18 L21 10 V20 H18 V17 H6 V20 H3 Z M3 10 H21 M6 13 H8 M16 13 H18',play:'M8 4 L20 12 8 20 Z',pause:'M8 4 V20 M16 4 V20',next:'M5 5 L15 12 5 19 Z M19 5 V19',prev:'M19 5 L9 12 19 19 Z M5 5 V19',temp:'M10 4 Q12 1 14 4 V14 A5 5 0 1 1 10 14 Z M12 7 V17',window:'M3 8 L8 3 H21 V20 H3 Z M6 9 L10 6 H18',sun:'M12 7 A5 5 0 1 1 11.99 7 M12 1 V4 M12 20 V23 M1 12 H4 M20 12 H23 M4 4 L6 6 M18 18 L20 20',heat:'M5 15 H20 V20 H3 V15 M7 12 Q3 10 7 7 Q10 4 7 2 M13 12 Q9 10 13 7 Q16 4 13 2 M19 12 Q15 10 19 7 Q22 4 19 2',wave:'M2 6 Q6 1 12 6 T22 6 M2 12 Q6 7 12 12 T22 12 M2 18 Q6 13 12 18 T22 18',volume:'M3 9 H7 L13 4 V20 L7 15 H3 Z M17 7 Q23 12 17 17',roof:'M3 17 L7 6 H21 L17 17 Z',shade:'M3 5 H21 M3 9 H21 M3 13 H21 M3 17 H21 M6 21 H18',lock:'M5 10 H19 V22 H5 Z M8 10 V6 A4 4 0 0 1 8 5 M16 10 V6 M12 14 V18',bulb:'M8 17 Q1 9 7 4 Q16 -1 20 8 Q22 13 16 17 V21 H8 Z',snow:'M12 2 V22 M3 7 L21 17 M3 17 L21 7 M9 4 L12 7 15 4 M9 20 L12 17 15 20',repeat:'M3 8 H20 L16 4 M21 16 H4 L8 20',shuffle:'M3 5 Q10 5 12 12 T21 19 M3 19 Q10 19 12 12 T21 5 M17 2 L21 5 17 8 M17 16 L21 19 17 22',check:'M4 12 L10 18 21 5',stop:'M5 5 H19 V19 H5 Z'};

const iconCache={};
paths.moon='M20 15 A9 9 0 1 1 9 3 A7 7 0 0 0 20 15 Z';

paths.headphones='M4 14 V11 A8 8 0 0 1 20 11 V14 M4 12 H7 V21 H4 Q2 21 2 19 V15 Q2 12 4 12 M20 12 H17 V21 H20 Q22 21 22 19 V15 Q22 12 20 12';
paths.allSeats='M4 3 H9 V10 H4 Z M15 3 H20 V10 H15 Z M4 14 H9 V21 H4 Z M15 14 H20 V21 H15 Z';
paths.Spotify='M22 12 A10 10 0 1 1 2 12 A10 10 0 1 1 22 12 M6 9 Q12 6.5 18 10 M7 12.5 Q12 10.5 17 13.5 M8 16 Q12 14.5 16 17';
paths.Radio='M4 8 L18 3 M3 8 H21 V21 H3 Z M6 12 H18 M14 16 H18 M14 18 H18 M10 17 A2 2 0 1 1 6 17 A2 2 0 1 1 10 17';
paths.Bluetooth='M7 6 L17 16 L12 21 V3 L17 8 L7 18';
function icon(name,color='#c0cbc5'){

 if(theme==='day'){if(['#05251b','#07271b','#08271c'].includes(color))color='#ffffff';if(['#c0cbc5','#8f9b9b','#b9c9c1','#9ca9a2'].includes(color))color='#50675b';else if(color==='#3ddc97')color='#315b70'}const key=name+color;if(!iconCache[key]){const c=document.createElement('canvas');c.width=c.height=72;const x=c.getContext('2d');x.scale(3,3);x.strokeStyle=color;x.lineWidth=1.55;x.lineCap='round';x.lineJoin='round';x.stroke(new Path2D(paths[name]||paths.Atmosphere));iconCache[key]=c.toDataURL();}

 return `<img class="icon" alt="" src="${iconCache[key]}">`;

}

const textCache=new Map();

function rasterize(root=document){root.querySelectorAll('[data-r]').forEach(el=>{const text=el.textContent,cs=getComputedStyle(el),font=`${cs.fontWeight} ${cs.fontSize} ${cs.fontFamily}`,spacing=cs.letterSpacing==='normal'?'0px':cs.letterSpacing,key=JSON.stringify([font,spacing,cs.color,text]);let o=textCache.get(key);if(!o){const c=document.createElement('canvas'),x=c.getContext('2d');x.font=font;x.letterSpacing=spacing;const w=Math.ceil(x.measureText(text).width)+2,h=Math.ceil(parseFloat(cs.fontSize)*1.4);c.width=w*2;c.height=h*2;x.scale(2,2);x.font=font;x.letterSpacing=spacing;x.fillStyle=cs.color;x.textBaseline='middle';x.fillText(text,0,h/2);o={src:c.toDataURL(),w,h};textCache.set(key,o)}el.innerHTML=`<img class="raster" src="${o.src}" width="${o.w}" height="${o.h}" alt="${text.replaceAll('"','&quot;')}">`;el.removeAttribute('data-r')})}

const btn=(label,action,cls='pill')=>`<button class="${cls}" data-action="${action}" aria-label="${label}">${T(label)}</button>`;

const toggle=(key,on)=>`<button class="switch ${on?'on':''}" data-toggle="${key}" role="switch" aria-checked="${on}" aria-label="${key==='ac'?'A/C':key==='auto'?'Auto climate':key}"><img src="assets/thumb.png" alt=""></button>`;

function slider(label,key,value,min=0,max=100,unit='%'){return `<div class="panel sliderbox"><div class="slidertitle">${T(label)}<span id="value-${key}">${T(value+unit)}</span></div><input type="range" aria-label="${label}" data-range="${key}" min="${min}" max="${max}" value="${value}" style="--pct:${(value-min)/(max-min)*100}%"></div>`}

function row(label,ico,control,detail=''){return `<div class="panel row">${icon(ico)}<div class="grow"><div class="label">${T(label)}</div>${detail?`<div class="detail">${T(detail)}</div>`:''}</div>${control}</div>`}

function dots(key,n){return `<div class="seat-levels ${key}" role="group" aria-label="${key==='heat'?'Seat heat':'Seat cooling'} level">${[0,1,2,3].map(i=>`<button class="pill ${i===n?'on':''}" data-level="${i}" data-action="level-${key}-${i}" aria-label="${key==='heat'?'Heat':'Cooling'} ${i===0?'off':'level '+i}" aria-pressed="${i===n}">${T(i===0?'Off':String(i))}</button>`).join('')}</div>`}


function title(text,sub='',control=''){return `<div class="title"><div><h2>${T(text)}</h2>${sub?`<div class="subtitle">${T(sub)}</div>`:''}</div>${control}</div>`}

function seatSelect(){return `<div class="seat-buttons" role="group" aria-label="Passenger seat">${['Rear left','Rear right'].map(v=>`<button class="seat-choice ${S.seat===v?'on':''}" data-action="seat-${v}" aria-label="${v}" aria-pressed="${S.seat===v}">${T(v)}</button>`).join('')}</div>`}

function scene(type='cabin'){return `<div class="scene ${type==='seat'?'seat':''}"><canvas data-seat="${S.seat}" id="sceneCanvas" width="540" height="360" aria-label="${type==='seat'?'Seat state preview':'Cabin state preview'}"></canvas>${type==='seat'?`<div class="seat-selector-overlay">${seatSelect()}</div>`:''}${type==='climate'?`<div class="climate-switches"><div>${icon('snow',S.ac?'#7dceff':'#8f9b9b')}<span class="ac-caption">${T('A/C')}<span>${T('Cool / dry')}</span></span>${toggle('ac',S.ac)}</div><div>${T('Auto')}${toggle('auto',S.auto)}</div></div>`:''}<div class="sceneLabel">${T(type==='seat'?'Comfort · '+S.seat:S.tab==='Atmosphere'?S.scene[0].toUpperCase()+S.scene.slice(1)+' · '+(S.beat?'Beat preview':'Ambient light'):S.seat+' · Live preview')}</div></div>`}

function experiences(names=['Aurora','Lounge','Relax']){return `<div class="experiencegrid">${names.map(n=>`<button class="experience ${S.scene===n.toLowerCase()?'on':''}" data-scene="${n.toLowerCase()}" aria-label="Activate ${n}"><img src="assets/presets/${n.toLowerCase()}.webp" alt=""><span>${T(n)}</span></button>`).join('')}</div>`}

const homeDateFormatter=new Intl.DateTimeFormat('en-US',{timeZone:'Europe/Helsinki',month:'long',day:'numeric'});
const homeDate=()=>homeDateFormatter.format(new Date());
function home(){const a=seat();return `<div class="home-hero">${scene()}<div class="hero-top">${title('Revontuli','',`<button class="avatar" data-action="profile" aria-label="About this preview">${T('S')}</button>`)}</div><div class="hero-weather"><div class="hero-date" data-date="${homeDate()}">${T(homeDate())}</div>${icon('snow','#b8f0da')}${T('Oulu · −4°')}<span class="small">${T('Light snow · sample')}</span></div><div class="hero-bottom"><span class="hero-connection"><span class="live"></span>${T('Preview connected')}</span>${seatSelect()}</div></div>`+`<div class="sectionlabel">${T('Your comfort')}</div><div class="grid"><div class="tile">${icon('temp','#3ddc97')}<div class="grow"><div class="meta">${T('Temperature')}</div><div class="value">${T(a.temp.toFixed(1)+'°')}</div></div><button class="step" data-action="temp-down" aria-label="Decrease temperature">−</button><button class="step" data-action="temp-up" aria-label="Increase temperature">+</button></div><button class="tile home-window" data-action="window-home" aria-label="${a.window>0?'Close window':'Open window'}">${icon('window',a.window>0?'#3ddc97':'#b9c9c1')}<div class="grow"><div class="meta">${T('Window')}</div><div class="home-window-state">${T(a.window===0?'Closed':a.window===100?'Open':Math.round(a.window)+'% open')}</div></div><span class="window-action">${T(a.window>0?'↑ Close':'↓ Open')}</span></button><button class="tile" data-action="cycle-heat">${icon('heat')}<div><div class="meta">${T('Seat heat')}</div><div class="value">${T(a.heat?'Level '+a.heat:'Off')}</div></div></button><button class="tile" data-tab="Atmosphere">${icon('sun','#3ddc97')}<div><div class="meta">${T('Lighting')}</div><div class="value" style="font-size:14px">${T(S.light?S.scene[0].toUpperCase()+S.scene.slice(1):'Off')}</div></div></button></div><div class="sectionlabel">${T('Experiences')}</div>`+experiences();}

function comfort(){const a=seat(),climate=S.comfortMode==='Climate';return title('Comfort','',climate?seatSelect():'')+`<div class="seg comfort-tabs">${['Climate','Seat'].map(v=>btn(v,'comfort-'+v,S.comfortMode===v?'on':'')).join('')}</div>`+scene(climate?'climate':'seat')+(climate?`<div class="panel temperature-control"><div class="temperature-caption">${T('Set temperature')}<span class="small">${T('Cabin 22° · demo')}</span></div><div class="temperature-adjust"><button class="step" data-action="temp-down" aria-label="Decrease temperature">−</button><strong class="temperature-target ${a.temp>22.25?'warm':a.temp<21.75?'cool':''}">${T(a.temp.toFixed(1)+'°')}</strong><button class="step" data-action="temp-up" aria-label="Increase temperature">+</button></div></div>`+slider('Fan speed','fan',a.fan,0,5,'')+`<div class="seg airflow-tabs">${['Face','Body','Feet'].map(n=>btn(n,'air-'+n,n===a.air?'on':'')).join('')}</div>`:row('Seat heat','heat',dots('heat',a.heat),['Off','Low','Medium','High'][a.heat])+row('Seat cooling','snow',dots('cool',a.cool),['Off','Low','Medium','High'][a.cool])+row('Massage','wave',toggle('massage',a.massage))+slider('Recline','recline',a.recline)+`<div class="seg">${btn('Save','save-position')}${btn('Recall','recall-position')}${btn('Reset','reset-seat')}</div>`)}
function atmosphere(){return title('Atmosphere','',`<div class="effect-control"><span class="small">${T('Effect')}</span><button class="switch ${S.light?'on':''}" role="switch" aria-label="Lighting effect" aria-checked="${S.light}" data-toggle="light"><img src="assets/thumb.png" alt=""></button></div>`)+`<div class="atmosphere-hero">${scene()}<div class="atmosphere-overlay"><div class="beat-control"><span>${T('Beat sync')}</span><button class="switch ${S.beat?'on':''}" role="switch" aria-label="Beat sync" aria-checked="${S.beat}" data-toggle="beat"><img src="assets/thumb.png" alt=""></button></div><div class="speed-control"><span class="small">${T('Speed')}</span><div class="seg" role="group" aria-label="Effect speed">${['Slow','Flow','Fast'].map(v=>btn(v,'speed-'+v,S.speed===v?'on':'')).join('')}</div></div></div></div>`+experiences(['Aurora','Lounge','Relax','Club','Cinema'])+slider('Intensity','brightness',S.brightness)+`<div class="sectionlabel">${T('Light zones')}</div><div class="seg">${['Doors','Roof','Footwell'].map(v=>btn(v,'zone-'+v,S.zones.includes(v)?'on':'')).join('')}</div>`}


function media(){return title('Media','A soundtrack for the journey')+`<div class="source-icons" role="group" aria-label="Music source">${['Spotify','Radio','Bluetooth'].map(v=>`<button class="source-icon ${S.source===v?'on':''}" data-action="source-${v}" aria-label="${v}" title="${v}" aria-pressed="${S.source===v}">${icon(v,S.source===v?'#05251b':'#b9c9c1')}</button>`).join('')}</div><div class="media-feature"><div class="media-feature-main"><img class="album ${S.playing?'playing':''}" src="assets/album.webp" alt="Green and violet aurora album artwork"><div class="tracktitle">${T(tracks[S.track])}</div><div class="artistline">${T('Kaamos')}</div></div><aside class="media-rail"><div class="output-buttons" role="group" aria-label="Audio output">${[['Cabin speakers','volume'],['Headphones','headphones']].map(([label,glyph])=>`<button class="output-button ${S.output===label?'on':''}" data-action="output-${label}" aria-label="${label}" title="${label}" aria-pressed="${S.output===label}">${icon(glyph,S.output===label?'#05251b':'#b9c9c1')}</button>`).join('')}</div><div class="focus-buttons" role="group" aria-label="Speaker focus">${['All seats','Front','Rear'].map(v=>`<button class="focus-button ${S.focus===v&&S.output==='Cabin speakers'?'on':''}" data-action="focus-${v}" aria-label="${v}" aria-pressed="${S.focus===v&&S.output==='Cabin speakers'}" ${S.output==='Headphones'?'disabled':''}>${v==='All seats'?icon('allSeats',S.focus===v&&S.output==='Cabin speakers'?'#05251b':'#b9c9c1')+T('All'):T(v)}</button>`).join('')}</div></aside></div><input type="range" aria-label="Playback position" data-range="progress" min="0" max="238" value="${S.progress}" style="--pct:${S.progress/238*100}%"><div class="times"><span id="elapsed">${T(time(S.progress))}</span>${T('3:58')}</div><div class="transport"><button data-toggle="repeat" aria-label="Repeat" aria-pressed="${S.repeat}">${icon('repeat',S.repeat?'#3ddc97':'#9ca9a2')}</button><button data-action="prev" aria-label="Previous track">${icon('prev')}</button><button class="primary" data-action="play" aria-label="${S.playing?'Pause':'Play'}">${icon(S.playing?'pause':'play','#07271b')}</button><button data-action="next" aria-label="Next track">${icon('next')}</button><button data-toggle="shuffle" aria-label="Shuffle" aria-pressed="${S.shuffle}">${icon('shuffle',S.shuffle?'#3ddc97':'#9ca9a2')}</button></div>`+`<div class="media-volume"><button data-action="mute" aria-label="${S.volume?'Mute':'Unmute'}" aria-pressed="${S.volume===0}">${icon('volume',S.volume?'#3ddc97':'#8f9b9b')}</button><span class="small">${T('Volume')}</span><input type="range" aria-label="Volume" data-range="volume" min="0" max="100" value="${S.volume}" style="--pct:${S.volume}%"><span id="value-volume">${T(S.volume+'%')}</span></div><div class="footnote">${T('Audio preview · services and vehicle audio not connected')}</div>`}


function cabin(){const a=seat();const active=windowMotion||(a.window===0?'close':a.window===100?'open':'stop');return `<div class="cabin-hero">${scene()}<div class="cabin-heading">${title('Cabin','',seatSelect())}</div></div>`+slider('Window · '+(windowMotion==='open'?'opening':windowMotion==='close'?'closing':a.window===0?'closed':a.window===100?'open':'partly open'),'window',Math.round(a.window))+`<div class="seg" style="margin-bottom:12px">${['open','stop','close'].map(v=>`<button data-action="window-${v}" class="${active===v?'on':''}" aria-pressed="${active===v}" aria-label="${v[0].toUpperCase()+v.slice(1)} window">${T(v[0].toUpperCase()+v.slice(1))}</button>`).join('')}</div>`+slider('Sunshade · closed','shade',a.shade)+`<div class="panel row"><div class="grow">${T('Sunroof')}</div><div class="seg">${['Closed','Tilt','Open'].map((v,i)=>btn(v,'roof-'+i,S.roof===i?'on':'')).join('')}</div></div><div class="thirds">${[['reading','Reading light','bulb'],['privacy','Privacy','lock'],['fresh','Fresh air','wave']].map(([k,n,ic])=>`<button class="panel" data-toggle="${k}" aria-pressed="${S[k]}" aria-label="${n}">${icon(ic,S[k]?'#3ddc97':'#8f9b9b')}${T(n)}<div class="small" style="margin-top:6px">${T(S[k]?'On':'Off')}</div></button>`).join('')}</div><div style="height:12px"></div>${btn('Close rear windows','close-all','pill wide')}`}

function time(n){return Math.floor(n/60)+':'+String(Math.floor(n%60)).padStart(2,'0')}

// Move raster assets without adding frames or idle animation loops.
const motionAllowed=()=>!matchMedia('(prefers-reduced-motion: reduce)').matches;
function microMotion(el,frames,duration=180){if(!el||!motionAllowed())return;el.getAnimations().forEach(a=>a.cancel());el.animate(frames,{duration,easing:'cubic-bezier(.2,.8,.2,1)'});}
function captureSwitches(){return new Map([...document.querySelectorAll('.switch[data-toggle]')].map(el=>[el.dataset.toggle,el.classList.contains('on')]));}
function restoreSwitchMotion(previous){document.querySelectorAll('.switch[data-toggle]').forEach(el=>{const before=previous.get(el.dataset.toggle),after=el.classList.contains('on');if(before!==undefined&&before!==after)microMotion(el.querySelector('img'),[{transform:'translateX('+(before?18:0)+'px)'},{transform:'translateX('+(after?18:0)+'px)'}]);});}

// Small raster skins reuse the existing nine-slice geometry in daylight.
function daySkin(color){const c=document.createElement('canvas');c.width=c.height=80;const x=c.getContext('2d');x.fillStyle=color;x.beginPath();x.roundRect(0,0,80,80,24);x.fill();return 'url('+c.toDataURL()+')';}
const daySkins={panel:daySkin('#f9faf4'),button:daySkin('#dce6dc'),dock:daySkin('#e3ebe0'),selected:daySkin('#315b70')};
function applyTheme(){
 document.body.dataset.theme=theme;const device=$('.device');device.dataset.theme=theme;
 for(const [name,url] of Object.entries(daySkins))device.style.setProperty('--day-'+name,url);
 $('#status').innerHTML='<span>19:04</span><button class="theme-toggle" data-action="theme" aria-label="Switch to '+(theme==='day'?'night':'day')+' mode" title="Switch to '+(theme==='day'?'night':'day')+' mode">'+icon(theme==='day'?'moon':'sun',theme==='day'?'#193e2e':'#e7f0e9')+'</button>';
}

function render(){applyTheme();const previousSwitches=captureSwitches();const sc=$('#screen'),old=sc.scrollTop;sc.dataset.tab=S.tab;$('.device').dataset.page=S.tab;sc.innerHTML=({Home:home,Comfort:comfort,Atmosphere:atmosphere,Media:media,Cabin:cabin})[S.tab]();sc.scrollTop=old;$('#nav').innerHTML=tabs.map(t=>`<button data-tab="${t}" class="${S.tab===t?'on':''}" aria-label="${t}" ${S.tab===t?'aria-current="page"':''}>${icon(t,S.tab===t?'#3ddc97':'#8f9b9b')}${T(t)}</button>`).join('');$('#player').style.display=['Media','Comfort','Cabin'].includes(S.tab)?'none':'flex';$('#player').innerHTML=`<img class="album-mini" src="assets/album.webp" alt=""><button class="grow" data-tab="Media" style="text-align:left"><div class="track">${T(tracks[S.track])}</div><div class="artist">${T('Kaamos')}</div></button><button class="play" data-action="play" aria-label="${S.playing?'Pause':'Play'}">${icon(S.playing?'pause':'play','#08271c')}</button><button data-action="next" aria-label="Next track">${icon('next')}</button>`;rasterize();lastPaint='';drawScene();restoreSwitchMotion(previousSwitches)}

function toast(s){$('#toast').textContent=s;$('#toast').classList.add('show');clearTimeout(toastTimer);toastTimer=setTimeout(()=>$('#toast').classList.remove('show'),2200)}

function img(src){if(!imageCache.has(src)){const im=new Image();im.onload=()=>{im.failed=false;lastPaint='';drawScene()};im.onerror=()=>{im.failed=true;lastPaint='';drawScene();if(!im.retried){im.retried=true;setTimeout(()=>{im.src=src+'?retry='+Date.now()},1500)}};imageCache.set(src,im);im.src=src}return imageCache.get(src)}

const frameCount=name=>name==='recline'?25:10;

function framePath(name,n){const id=String(Math.max(0,Math.min(frameCount(name)-1,Math.round(n)))).padStart(2,'0');return name==='recline'?`assets/sequences/recline-jpeg/seat_${id}.jpg`:`assets/sequences/${name}/${id}.webp`}

function frame(name,n){return img(framePath(name,n))}

// Small cached raster overlays keep the original seat photography unchanged.
const seatFxCache=new Map();
let seatFxTick=0;
function seatThermalFx(kind,level,index){
 const key=kind+level+index;if(seatFxCache.has(key))return seatFxCache.get(key);
 const c=document.createElement('canvas');c.width=270;c.height=180;const g=c.getContext('2d');g.scale(.5,.5);
 g.lineCap='round';g.lineJoin='round';g.lineWidth=3.5;
 const progress=index/10;
 g.strokeStyle=kind==='heat'?'#ffe3a8':'#d5f6ff';g.shadowColor=kind==='heat'?'#ff9f38':'#53cfff';g.shadowBlur=7;
 g.globalAlpha=.55+.4*Math.sin(Math.PI*progress);
 for(let lane=0;lane<level;lane++){
  const offset=(lane-(level-1)/2)*27;
  if(kind==='heat'){
   // Warmth rises from the upholstery in soft, curved ribbons.
   for(const base of [[295,197],[232,282]]){
    const x=base[0]+offset,y=base[1]-progress*18;g.beginPath();g.moveTo(x,y);
    g.bezierCurveTo(x-12,y-12,x+12,y-24,x,y-39);g.stroke();
   }
  }else{
   // Ventilation leaves the backrest and cushion: short directional streams.
   for(const base of [[304,160],[237,266]]){
    const x=base[0]+offset-progress*12,y=base[1]+progress*8;
    g.beginPath();g.moveTo(x+12,y-17);g.quadraticCurveTo(x-3,y-12,x-19,y+6);
    g.moveTo(x-19,y+6);g.lineTo(x-19,y-3);g.moveTo(x-19,y+6);g.lineTo(x-10,y+5);g.stroke();
   }
  }
 }
 seatFxCache.set(key,c);return c;
}

// An inactive effect must not hide another seat setting that is still on.
function resolveSeatPreview(a,preferred){
 if(preferred==='recline'||(['heat','cool','massage'].includes(preferred)&&a[preferred]))return preferred;
 return a.cool?'cool':a.heat?'heat':a.massage?'massage':'recline';
}

function drawScene(){const canvas=$('#sceneCanvas');if(!canvas)return;const a=seat(),x=canvas.getContext('2d');if(S.tab==='Comfort'&&S.comfortMode==='Climate'){drawClimate(canvas,x,a);return}const phase=Math.round((1-Math.cos(tick*.35))*4.5),lightLevel=S.light?S.brightness/100*9:0;const key=JSON.stringify([S.tab,a,S.roof,S.scene,lightLevel,S.beat,S.zones,S.seatPreview,S.reading,phase,Math.floor(tick)%10,seatFxTick]);if(key===lastPaint)return;

 let n=resolveSeatPreview(a,S.seatPreview),v=['heat','cool'].includes(n)?a[n]*3:n==='massage'?(a.massage?phase:0):a[n]/100*(frameCount(n)-1);

 const power=S.beat?lightLevel*(.4+.6*phase/9):lightLevel;

 const required=S.tab==='Comfort'?[frame(n,v)]:[img('assets/cabin.webp'),frame('window',a.window/100*9),frame('roof',S.roof===0?0:S.roof===1?3:9),...(a.shade>0?[frame('shade',9)]:[]),...(S.light?[frame('light-'+S.scene,power)]:[])];

 let status=canvas.parentElement.querySelector('.load-status');if(!status){status=document.createElement('div');status.className='load-status';status.setAttribute('role','status');canvas.parentElement.append(status)}

 if(required.some(im=>!im.complete||!im.naturalWidth)){status.textContent=required.some(im=>im.failed)?'Preview images unavailable — check the local server':'Loading animation frames…';status.hidden=false;return}status.hidden=true;lastPaint=key;

 // Fetch the current action's ten frames before the next interaction, preserving the last painted frame while loading.

 const preload=S.tab==='Comfort'?[n]:['window','shade','roof',...(S.light?['light-'+S.scene]:[])];preload.forEach(name=>{for(let i=0;i<frameCount(name);i++)frame(name,i)});

 canvas.dataset.sequence=S.tab==='Comfort'?n:'cabin';canvas.dataset.frame=String(Math.round(S.tab==='Comfort'?v:a.window/100*9));canvas.dataset.lightFrame=String(Math.round(power));x.clearRect(0,0,540,360);

 const draw=(im)=>{if(im.complete&&im.naturalWidth)x.drawImage(im,0,0,540,360)};

 if(S.tab==='Comfort'){const im=frame(n,v);if(n==='recline'){x.fillStyle='#000';x.fillRect(0,0,540,360);x.drawImage(im,90,0,360,360)}else draw(im);if((n==='heat'||n==='cool')&&a[n]>0)x.drawImage(seatThermalFx(n,a[n],seatFxTick),0,0,540,360);return}

 draw(img('assets/cabin.webp'));
 if(S.light){const power=S.beat?lightLevel*(.4+.6*phase/9):lightLevel;x.save();x.beginPath();if(S.zones.includes('Doors'))x.rect(0,72,540,213);if(S.zones.includes('Footwell'))x.rect(0,285,540,75);if(S.zones.includes('Roof'))x.rect(0,0,540,72);x.clip();draw(frame('light-'+S.scene,power));x.restore()}

 x.save();x.beginPath();x.moveTo(38,165);x.lineTo(48,78);x.lineTo(68,60);x.lineTo(354,60);x.lineTo(370,73);x.lineTo(353,164);x.closePath();x.clip();draw(frame('window',a.window/100*9));if(a.shade>0){const y=165-105*a.shade/100;x.beginPath();x.rect(0,y,540,360-y);x.clip();draw(frame('shade',9))}x.restore();

 x.save();x.beginPath();x.rect(0,0,540,47);x.clip();draw(frame('roof',S.roof===0?0:S.roof===1?3:9));x.restore();



 if(S.tab==='Home'&&a.heat){const heat=frame('home-heat',a.heat*3);if(heat.complete&&heat.naturalWidth)x.drawImage(heat,0,0,540,360);const name='home-heat-fx-'+a.heat;const fx=frame(name,Math.floor(tick)%10);if(fx.complete&&fx.naturalWidth)x.drawImage(fx,0,0,540,360);for(let i=0;i<10;i++)frame(name,i)}
 if(S.reading){x.save();const g=x.createRadialGradient(425,150,0,425,150,120);g.addColorStop(0,'#ffdb8e33');g.addColorStop(1,'#ffdb8e00');x.fillStyle=g;x.fillRect(0,0,540,360);x.restore()}

}

function applySeatCommand(key,value){
 const a=seat(),core=globalThis.revontuliSeatCore;
 if(!core){a[key]=value;if(value&&(key==='heat'||key==='cool'))a[key==='heat'?'cool':'heat']=0;return}
 const result=core._seat_command(S.seat==='Rear left'?0:1,['heat','cool','massage','recline'].indexOf(key),Number(value));
 if(result<0)return;
 Object.assign(a,{heat:result&3,cool:(result>>2)&3,massage:!!(result&16),recline:result>>5});
 if(window.parent!==window)window.parent.dispatchEvent(new CustomEvent('revontuli-seat',{detail:{key,seat:S.seat,heat:a.heat,cool:a.cool,massage:a.massage,recline:a.recline}}));
}
function windowMove(to){clearInterval(windowTimer);windowMotion=to?'open':'close';const targetSeat=seat();render();windowTimer=setInterval(()=>{const diff=to-targetSeat.window;targetSeat.window=Math.max(0,Math.min(100,targetSeat.window+Math.sign(diff)*Math.min(5,Math.abs(diff))));const range=$('[data-range="window"]'),value=$('#value-window');if(range){range.value=targetSeat.window;range.style.setProperty('--pct',targetSeat.window+'%')}if(value){value.innerHTML=T(Math.round(targetSeat.window)+'%');rasterize(value)}const homeState=$('.home-window-state');if(homeState){homeState.innerHTML=T((to?'Opening ':'Closing ')+Math.round(targetSeat.window)+'%');rasterize(homeState)}lastPaint='';drawScene();if(targetSeat.window===to){clearInterval(windowTimer);windowMotion=null;if(S.tab==='Cabin'||S.tab==='Home')render();toast(to?'Window open · simulated':'Window closed · simulated')}},80)}

document.addEventListener('click',e=>{const b=e.target.closest('button');if(!b)return;const a=seat();if(b.dataset.tab){if(S.tab===b.dataset.tab)return;S.tab=b.dataset.tab;$('#screen').scrollTop=0;render();microMotion($('#screen'),[{opacity:.65,transform:'translateY(4px)'},{opacity:1,transform:'translateY(0)'}],180);microMotion($('#nav .on .icon'),[{transform:'translateY(2px)'},{transform:'translateY(-1px)'}],180);return}if(b.dataset.scene){let n=b.dataset.scene;if(n==='relax'){S.scene='relax';S.brightness=55;S.beat=false;applySeatCommand('massage',true);S.seatPreview='massage';toast('Relax · soft cyan light and gentle massage')}else{S.scene=n;S.brightness=({aurora:85,lounge:70,club:100,cinema:35})[n]||65;S.beat=n==='club'}S.light=true;render();return}if(b.dataset.toggle){const k=b.dataset.toggle;if(k==='massage'){applySeatCommand('massage',!a.massage);S.seatPreview='massage'}else{S[k]=!S[k];if(k==='ac')S.auto=false;if(k==='auto'&&S.auto){S.ac=true;if(!a.fan)a.fan=3}if(k==='privacy')a.shade=S.privacy?100:0}render();return}const act=b.dataset.action;if(!act)return;if(act==='theme'){theme=theme==='day'?'night':'day';try{localStorage.setItem('revontuli-theme',theme)}catch{}render();return}if(act.startsWith('level-')){const [,key,level]=act.split('-');applySeatCommand(key,Number(level));S.seatPreview=key;render();return}if(act.startsWith('seat-')){clearInterval(windowTimer);windowMotion=null;const changed=S.seat!==act.slice(5);S.seat=act.slice(5);render();if(changed&&S.tab==='Comfort'&&S.comfortMode==='Seat'&&!matchMedia('(prefers-reduced-motion: reduce)').matches){$('#sceneCanvas').animate([{opacity:0},{opacity:1}],{duration:260,easing:'ease-out'})}return}if(act.startsWith('output-')){S.output=act.slice(7);render();return}if(act==='mute'){if(S.volume){S.lastVolume=S.volume;S.volume=0}else S.volume=S.lastVolume||45;render();return}if(act.startsWith('focus-')){S.focus=act.slice(6);render();return}

 if(act.startsWith('comfort-')){S.comfortMode=act.slice(8);S.seatPreview='recline';$('#screen').scrollTop=0}
 else if(act==='temp-up'||act==='temp-down')a.temp=Math.max(16,Math.min(30,a.temp+(act==='temp-up'?.5:-.5)));

 else if(act.startsWith('cycle-')){const k=act.slice(6);applySeatCommand(k,(a[k]+1)%4);S.seatPreview=k}

 else if(act==='play')S.playing=!S.playing;

 else if(act==='next'||act==='prev'){S.track=(S.track+(act==='next'?1:2))%3;S.progress=0}

 else if(act==='window-open'||act==='window-close'||act==='window-home'){windowMove(act==='window-open'?100:act==='window-close'?0:a.window>0?0:100);return}

 else if(act==='window-stop'){clearInterval(windowTimer);windowMotion=null;toast('Window stopped · simulated')}

 else if(act==='close-all'){clearInterval(windowTimer);windowMotion=null;Object.values(S.seats).forEach(v=>v.window=0);toast('Rear windows closed · simulated')}

 else if(act.startsWith('roof-'))S.roof=Number(act.slice(5));

 else if(act.startsWith('speed-'))S.speed=act.slice(6);

 else if(act.startsWith('air-')){a.air=act.slice(4);S.auto=false;toast('Airflow directed to '+a.air.toLowerCase())}

 else if(act.startsWith('source-')){S.source=act.slice(7);toast(S.source+' · demo source')}

 else if(act.startsWith('zone-')){const z=act.slice(5);S.zones=S.zones.includes(z)?S.zones.filter(v=>v!==z):[...S.zones,z]}

 else if(act==='stop-effect'){S.beat=false;S.light=false}

 else if(act==='save-position'){try{localStorage.setItem('revontuli-seat-'+S.seat,JSON.stringify({recline:a.recline}));toast('Seat position saved on this device')}catch{toast('Storage is unavailable')}}

 else if(act==='recall-position'){try{const saved=JSON.parse(localStorage.getItem('revontuli-seat-'+S.seat)||'null');if(saved&&Number.isFinite(saved.recline)){applySeatCommand('recline',Math.round(Math.max(0,Math.min(100,saved.recline))));S.seatPreview='recline';toast('Saved recline restored')}else toast('No saved position for this seat')}catch{toast('Saved position unavailable')}}
 else if(act==='reset-seat'){S.seats[S.seat]=initialSeat();if(globalThis.revontuliSeatCore){applySeatCommand('heat',2);applySeatCommand('massage',false);applySeatCommand('recline',0)}S.seatPreview='heat';toast('Comfort settings reset')}

 else if(act==='profile')toast('Revontuli · local image-based prototype');

 render();

});

document.addEventListener('input',e=>{if(!e.target.dataset.range)return;const k=e.target.dataset.range,v=Number(e.target.value);if(k in seat()){if(k==='recline')applySeatCommand(k,v);else seat()[k]=v;if(k==='fan')S.auto=false;if(k==='recline')S.seatPreview=k;if(k==='heat'||k==='cool')S.seatPreview=k;if(k==='window'){clearInterval(windowTimer);windowMotion=null}}else S[k]=v;e.target.style.setProperty('--pct',((v-Number(e.target.min))/(Number(e.target.max)-Number(e.target.min))*100)+'%');const label=$('#value-'+k);if(label){label.innerHTML=T(v+(k==='fan'?'':'%'));rasterize(label)}if(k==='progress'){$('#elapsed').innerHTML=T(time(v));rasterize($('#elapsed'))}lastPaint='';drawScene()});

document.addEventListener('change',e=>{if(e.target.dataset.range)render();});

setInterval(()=>{if(matchMedia('(prefers-reduced-motion: reduce)').matches)return;tick+=S.speed==='Slow'?.4:S.speed==='Fast'?1.8:1;seatFxTick=(seatFxTick+1)%10;drawScene()},100);

setInterval(()=>{if(!S.playing)return;S.progress++;if(S.progress>238){S.progress=0;if(!S.repeat)S.track=S.shuffle?Math.floor(Math.random()*3):(S.track+1)%3;render()}if(S.tab==='Media'){const inp=$('[data-range="progress"]');if(inp){inp.value=S.progress;inp.style.setProperty('--pct',S.progress/238*100+'%')}$('#elapsed').innerHTML=T(time(S.progress));rasterize($('#elapsed'))}},1000);

$('#gallerySummary').textContent=seqNames.length+' sequences · '+seqNames.reduce((total,name)=>total+frameCount(name),0)+' frames · 540 × 360 scenes / 240 × 240 recline';
$('#sequenceSelect').innerHTML=seqNames.map(n=>`<option>${n}</option>`).join('');

function galleryUpdate(){const n=$('#sequenceSelect').value,count=frameCount(n);$('#frameRange').max=count-1;const i=Math.min(count-1,Number($('#frameRange').value));$('#frameRange').value=i;$('#galleryFrame').src=framePath(n,i);$('#galleryFrame').style.objectFit='contain';$('#galleryFrame').style.maxHeight='360px';$('#frameLabel').textContent=`Frame ${i+1} / ${count} · ${n} · ${n==='recline'?'240 × 240 JPEG':'540 × 360 WebP'}`}

$('#galleryButton').onclick=()=>{$('#gallery').showModal();galleryUpdate()};$('#closeGallery').onclick=()=>{$('#gallery').close();clearInterval(galleryTimer);galleryTimer=null};$('#frameRange').oninput=galleryUpdate;$('#sequenceSelect').onchange=()=>{clearInterval(galleryTimer);galleryTimer=null;$('#frameRange').value=0;galleryUpdate()};$('#playSequence').onclick=()=>{if(galleryTimer){clearInterval(galleryTimer);galleryTimer=null;return}let d=1;galleryTimer=setInterval(()=>{let i=Number($('#frameRange').value)+d;if(i>=frameCount($('#sequenceSelect').value)-1||i<=0)d=-d;$('#frameRange').value=i;galleryUpdate()},120)};

window.revontuli={state:S,render,sequenceNames:seqNames};render();


function drawClimate(canvas,x,a){
 if(canvas.height!==270)canvas.height=270;
 const index=a.fan?Math.floor(tick*(.25+a.fan*.2))%10:0;
 const thermal=a.temp>22.25?'warm':a.temp<21.75&&S.ac?'cool':'neutral';
 const sequence=thermal==='neutral'?'airflow-'+a.air.toLowerCase()+'-vent':'climate-'+thermal+'-'+a.air.toLowerCase();
 const path=a.fan?`assets/sequences/${sequence}/${String(index).padStart(2,'0')}.webp`:'assets/climate.webp';
 const im=img(path);
 if(im.complete&&im.naturalWidth){x.clearRect(0,0,540,270);x.drawImage(im,0,90,540,270,0,0,540,270);canvas.dataset.sequence=a.fan?sequence:'off';canvas.dataset.frame=String(index)}
 for(let i=0;i<10;i++)img(`assets/sequences/${sequence}/${String(i).padStart(2,'0')}.webp`);
 const label=canvas.parentElement.querySelector('.sceneLabel');label.textContent=a.fan?`${a.air==='Feet'?'Floor outlets':'Rear console vent'} · ${a.air.toLowerCase()} · fan ${a.fan}`:'Airflow off';
}

// Refresh the date across midnight without redrawing the controls.
setInterval(()=>{const el=$('.hero-date'),date=homeDate();if(el&&el.dataset.date!==date){el.dataset.date=date;el.innerHTML=T(date);rasterize(el)}},30000);
