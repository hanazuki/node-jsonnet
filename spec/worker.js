import { Jsonnet } from '@hanazuki/node-jsonnet';
import { workerData, parentPort } from 'node:worker_threads';

const { extVars = {}, snippet } = workerData;
const jsonnet = new Jsonnet().extString(extVars);
const result = await jsonnet.evaluateSnippet(snippet);
parentPort.postMessage(result);
