// SPDX-License-Identifier: MIT
// TypeScript Version: 3.1

export type JsonnetPrimitiveValue = null | boolean | number | string;

export class Jsonnet {
  /**
   * Returns the version of linked libjsonnet.
   */
  static readonly version: string;

  /**
   * Constructs a new Jsonnet interpreter.
   */
  constructor();

  /**
   * Set the maximum stack depth.
   *
   * @param value - The stack depth.
   */
  maxStack(value: number): this;

  /**
   * Set the number of lines of back trace to display.
   *
   * @param value - The number of lines.
   */
  maxTrace(value: number): this;

  /**
   * Set the minimal number of objects required to trigger a garbage collection.
   *
   * @param value - The number of objects.
   */
  maxGcMinObjects(value: number): this;

  /**
   * Set the growth rate of the number of objects to trigger a garbage collection.
   *
   * @param value - The growth rate.
   */
  maxGcGrowthTrigger(value: number): this;

  /**
   * Binds external variable named as `key` to string `value`.
   *
   * @param key - Name of the external variable
   * @param value - String value to which the variable is bound
   */
  extString(key: string, value: string): this;

  /**
   * Binds external variable named as `key` to a value represented by `value`.
   *
   * @param key - Name of the external variable
   * @param value - Code representation of the value to which the variable is bound
   */
  extCode(key: string, value: string): this;

  /**
   * Binds top-level argument named as `key` to string `value`.
   *
   * @param key - Name of the top-level argument
   * @param value - String value to which the argument is bound
   */
  tlaString(key: string, value: string): this;

  /**
   * Binds top-level argument named as `key` to a value represented by `value`.
   *
   * @param key - Name of the top-level argument
   * @param value - Code representation of the value to which the argument is bound
   */
  tlaCode(key: string, value: string): this;

  /**
   * Adds a path to the list of include paths. Paths added more recently take precedence.
   *
   * @param path - Path to add.
   */
  addJpath(path: string): this;

  /**
   * Registers a native callback function.
   *
   * @example
   * ```typescript
   * const jsonnet = new Jsonnet();
   * jsonnet.nativeCallback("add", (a, b) => Number(a) + Number(b), "a", "b");
   * jsonnet.evaluateSnippet('std.native("add")(2, 3)')
   *        .then(result => console.log(JSON.parse(result)));  // => 5
   * ```
   *
   * @bug If the native callback throws an error, the entire Node.js process panics.
   *   To avoid this, use async function for operations that may throw.
   * ```typescript
   * // Node.js crashes
   * jsonnet.nativeCallback("failBad", () => { throw "fail"; });
   * jsonnet.evaluateSnippet(`std.native("failBad")()`);
   *
   * // OK
   * jsonnet.nativeCallback("failGood", async () => { throw "fail"; });
   * jsonnet.evaluateSnippet(`std.native("failGood")()`)
   *        .catch(error => console.log(error));  // => "fail"
   * ```
   *
   * @param name - Name of the callback function.
   * @param fun - The function to register as a callback.
   *   `fun` can return an Object as the callback result or a Promise for it.
   *  Since the result will be serialized as JSON before passed back to Jsonnet,
   *  it cannot contain any recursive structures.
   * @param params - Names of the function parameters.
   *   `params` must have the equal number of elements as the number of the parameters of `fun`.
   */
  nativeCallback<T extends string[]>(
    name: string,
    fun: (...args: JsonnetPrimitiveValue[] & { [K in keyof T]: JsonnetPrimitiveValue }) => any,
    ...params: T): this;

  /**
   * Evaluates a Jsonnet script in a file.
   *
   * @param filename - Path to the file.
   * @returns A Promise for the evaluation result in JSON representation.
   */
  evaluateFile(filename: string): Promise<string>;

  /**
   * Evaluates a Jsonnet script given as a string.
   *
   * @param snippet - Jsonnet script to evaluate.
   * @param filename - Filename used for error reporting.
   * @returns A Promise for the evaluation result in JSON representation.
   */
  evaluateSnippet(snippet: string, filename?: string): Promise<string>;

  /**
   * Evaluates a Jsonnet script in a file and returns a number of named JSON's.
   *
   * @param filename - Path to the file.
   * @returns A Promise for the evaluation result in JSON representation.
   */
  evaluateFileMulti(filename: string): Promise<{[name: string]: string}>;

  /**
   * Evaluates a Jsonnet script given as a string and returns a number of named JSON's.
   *
   * @param snippet - Jsonnet script to evaluate.
   * @param filename - Filename used for error reporting.
   * @returns A Promise for the evaluation result in JSON representation.
   */
  evaluateSnippetMulti(snippet: string, filename?: string): Promise<{[name: string]: string}>;

  /**
   * Evaluates a Jsonnet script in a file and returns a number of JSON's.
   *
   * @param filename - Path to the file.
   * @returns A Promise for the evaluation result in JSON representation.
   */
  evaluateFileStream(filename: string): Promise<string[]>;

  /**
   * Evaluates a Jsonnet script given as a string and returns a number of JSON's.
   *
   * @param snippet - Jsonnet script to evaluate.
   * @param filename - Filename used for error reporting.
   * @returns A Promise for the evaluation result in JSON representation.
   */
  evaluateSnippetStream(snippet: string, filename?: string): Promise<string[]>;
}
