# usage: python3 mk_page3.py   -- joins the shared constants of apply3.js (from "const MAIN" to just before "const FORMAT") with page3.body.js -> page3.js
import os, re
here = os.path.dirname(os.path.abspath(__file__))
a = open(os.path.join(here, 'apply3.js'), encoding='utf-8').read()
i = a.index('const MAIN = '); j = a.index('const FORMAT = ')
meta = """export const meta = {
  name: 'rta-1009-page3',
  description: 'Audio-DNA s-rta-1009: the list for Boris\\'s page 3. One ruling across the topics writes the answers he is owed and one item per assumption his answers opened; six checkers read it, each through one lens (his words, coverage, plain words, his chair, logic, the truth of the answers); a second ruling writes edit blocks. Paper only: nothing is built.',
  phases: [{ title: 'List' }, { title: 'Check' }, { title: 'Rule' }],
}
"""
open(os.path.join(here, 'page3.js'), 'w', encoding='utf-8').write(meta + a[i:j] + open(os.path.join(here, 'page3.body.js'), encoding='utf-8').read())
print('page3.js written')
