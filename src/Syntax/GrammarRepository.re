/*
 * GrammarRepository.re
 */

type t = {
  scopeToGrammar: Hashtbl.t(string, Textmate.Grammar.t),
  grammarInfo: Exthost.GrammarInfo.t,
  log: string => unit,
};

let create = (~log=_ => (), grammarInfo) => {
  log,
  scopeToGrammar: Hashtbl.create(32),
  grammarInfo,
};

let empty = create(Exthost.GrammarInfo.initial);

let getGrammar = (~scope: string, gr: t) => {
  switch (Hashtbl.find_opt(gr.scopeToGrammar, scope)) {
  | Some(v) => Some(v)
  | None =>
    switch (
      Exthost.GrammarInfo.getGrammarPathFromScope(gr.grammarInfo, scope)
    ) {
    | Some(grammarPath) =>
      gr.log("Loading grammar from: " ++ grammarPath);

      switch (Yojson.Safe.from_file(grammarPath)) {
      | json =>
        switch (Textmate.Grammar.Json.of_yojson(json)) {
        | Ok(grammar) =>
          gr.log("JSON Grammar loaded successfully");
          Hashtbl.add(gr.scopeToGrammar, scope, grammar);
          Some(grammar);

        | Error(e) =>
          gr.log("Grammar loading failed with: " ++ e);
          None;
        }

      | exception (Yojson.Json_error(_)) =>
        switch (Textmate.Grammar.Xml.of_file(grammarPath)) {
        | Ok(grammar) =>
          gr.log("XML Grammar loaded successfully");
          Hashtbl.add(gr.scopeToGrammar, scope, grammar);
          Some(grammar);

        | Error(e) =>
          gr.log("Grammar loading failed with: " ++ e);
          None;
        }
      };

    | None => None
    }
  };
};
