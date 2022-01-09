const divisonTypeMask = 1<<15;
const variableMask = 0x7f;
const varDoneMask = 1<<7;

const seq = (...parsers) => {
  let idx = 0;
  return (char, ctx) => {
    if (!parsers[idx]) return { d: true };
    const { d } = parsers[idx](char, ctx);
    return { d: d && (++idx >= parsers.length) }
  }
}

const repeat = (n, parser, ...args) => {
  let p = parser.apply(this, args), c = 0
  return (char, ctx) => {
    const { d } = p(char, ctx);
    if (++c >= n) return { d: true }
    if (d) p = parser.apply(this, args)
    return { d: false }
  }
}

const skip = (n) => {
  let c = 0;
  return () => ({d: ++c >= n })
}

const readString = (n) => {
  let str = '';
  return (char) => ({v: (str=str+String.fromCharCode(char)), d: str.length >= n });
}

const printString = (n) => {
  const p = readString(n);
  return (char) => { const {v,d} = p(char); if (d) console.log(v); return {v,d}};
}


const readNum = (n) => {
  let x = 0, c = 0;
  return (char) => ({v: (x=(x<<8)+char), d: ++c >= n })
}

const readVarnum = () => {
  let x = 0;
  return (char) => {
    x=(x<<7)+(char&variableMask);
    return {v: x, d: !(char&varDoneMask)}
  }
}

const set = (k, parser, ...args) => {
  const p = parser.apply(this, args);
  return (char, ctx) => {
    const r = p(char);
    if (r.d) ctx[k] = r.v
    return r
  }
}

const expectString = (name, expected) => {
  const p = readString(expected.length)
  return (char) => {
    const { v, d } = p(char);
    if (d && v != expected) throw Error(`expected ${name} to be "${expected}" but was "${v}"`);
    return { v, d }
  }
}

const expectNum = (name, n, expected) => {
  const p = readNum(n)
  return (char) => {
    const { v, d } = p(char);
    if (d && v != expected) throw Error(`expected ${name} to be ${expected} but was ${v}`);
    return { v, d }
  }
}

const ticksToMs = (ctx, ticks) => Math.floor(ticks * ctx.msPerTick)

const startNote = (ctx, key) => {
  // if (ctx.notes.length<100) console.log('start', key, ticksToMs(ctx, ctx.noteDelta))
  if (ctx.currentNote === key)  return;
  if (ctx.noteDelta) {
    // end previous rest
    if (ctx.notes.length<100) console.log('note', ctx.currentNote||0, ctx.noteDelta)
    ctx.notes.push( { key: ctx.currentNote||0, ms: ticksToMs(ctx, ctx.noteDelta) })
  } else if (ctx.currentNote) {
    // end previous note
    endNote(ctx, ctx.currentNote);
  }

  ctx.noteDelta = 0;
  ctx.currentNote = key;
}

const endNote = (ctx, key) => {
  // if (ctx.notes.length<100) console.log('end', key, ctx.noteDelta, ctx.currentNote, ctx.deltaTicks)
  // only end notes that are currently playing
  if (ctx.currentNote !== key) return;

  if (ctx.noteDelta) {
    if (ctx.notes.length<100) console.log('note', key, ctx.noteDelta)
    ctx.notes.push( { key, ms: ticksToMs(ctx, ctx.noteDelta) })
  }

  ctx.noteDelta = 0;
  ctx.currentNote = 0;
}

const noteOn = () => {
  let key;
  return (char, ctx) => {
    if (key == undefined) {
      key = char;
      return { d: false };
    }
    // if (ctx.notes.length<100) console.log('noteOn', key, ctx.deltaTicks, char, ctx.noteDelta)
    char > 0 ? startNote(ctx, key) : endNote(ctx, key);
    return { d: true };
  }
}

const noteOff = () => {
  let key;
  return (char, ctx) => {
    if (key == undefined) {
      key = char;
      return { d: false };
    }
    // if (ctx.notes.length<100) console.log('noteOff', key, ctx.deltaTicks, ctx.noteDelta)
    endNote(ctx, key);
    return { d: true };
  }
}

const parseTempo = () => {
  const p = readNum(3);
  return (char, ctx) => {
    const { v, d } = p(char, ctx);
    if (d) {
      ctx.msPerQNote = v / 1000;
      computeMsPerTick(ctx);
    }
    return { d };
  }
}

const meta = () => {
  let type, len, p;
  return (char, ctx) => {
    if (p) return p(char, ctx);

    if (type === undefined) type = char;
    else if (len === undefined) {
      len = char;
      if (type == 0x51) p = parseTempo();
      else if (len == 0) return { d: true }
      else p = printString(len);
    }
    return { d: false };
  }
}

const eventPayloadParser = (type) => {
  switch (type) {
    case 0x8: return noteOff();
    case 0x9: return noteOn();
    case 0xa: return skip(2);
    case 0xb: return skip(2);
    case 0xc: return skip(1);
    case 0xd: return skip(1);
    case 0xe: return skip(2);
    case 0xf: return meta();
  }
  throw Error(`Unknown control type ${type.toString(16)}`)
}

const eventPayload = () => {
  let p;
  return (char, ctx) => {
    if (p) {
      const { v, d } = p(char, ctx);
      return { v, d }
    } else {
      // add tick delta to note duration
      ctx.noteDelta = (ctx.noteDelta||0) + ctx.deltaTicks;
    }

    if (char & (1<<7)) {
      // new control type
      const controlType = char>>4;
      if (char != 0xff) ctx.controlType = controlType;
      p = eventPayloadParser(controlType);
      return { d: false }
    }

    // inherit previous control type
    if (!ctx.controlType) throw Error("midi message inherits control type, but no previous type found");
    p = eventPayloadParser(ctx.controlType);
    return p(char, ctx);
  }
}

const midiEvent = () => seq(set('deltaTicks', readVarnum), eventPayload())

const trackBody = () => {
  let p;
  return (char, ctx) => {
    if (!p) {
      p = repeat(ctx.trackLength, midiEvent)
      ctx.noteDelta = 0;
      ctx.controlType = undefined;
    }
    return p(char, ctx)
  }
}

const computeMsPerTick = (ctx) => {
  if (ctx.smpte) return;
  ctx.msPerTick = ctx.msPerQNote /*ms/qn*/ / ctx.ticksPerQNote /*qn/t*/;
}

const parseDivision = () => {
  const p = readNum(2);
  return (char, ctx) => {
    const { v, d } = p(char);
    if (d) {
      if (v & divisonTypeMask) {
        const frames = ((~v>>8) & 0xff) + 1;
        const divisions = v & 0xff;
        ctx.smpte = true
      } else {
        ctx.ticksPerQNote = v
        computeMsPerTick(ctx);
      }
    }
    return { d };
  }
}

const parseHeader = () => seq(
  expectString('header marker', 'MThd'),
  expectNum('header length', 4, 6),
  skip(2),
  set('numTracks', readNum, 2),
  parseDivision(),
)

const parseTrack = () => seq(
  expectString('track marker', 'MTrk'),
  set('trackLength', readNum, 4),
  trackBody(),
)

const parseTracks = () => {
  let p;
  return (char, ctx) => {
    if (!p) p = seq(...(Array(ctx.numTracks).fill().map(()=>parseTrack())))
    return p(char, ctx);
  }
}

const start = () => seq(parseHeader(), parseTracks())

const parse = () => {
  const ctx = { msPerQNote: 500, notes: [] }, p = start();
  return {
    parse: (char) => p(char, ctx),
    result: ctx
  }
}

export default parse;
