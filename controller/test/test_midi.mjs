import midiParser from './data/midi.js';

const parser = midiParser();

process.stdin.on('data', (buf) => {
  for (let i=0; i<buf.length; i++) parser.parse(buf[i]);
});

process.stdin.on('end', (buf) => {
  console.log("midi file parsed");
  console.log(parser.result)
});