#!/usr/bin/env python3
"""Verify FST enum metadata and waveforms (make check-installed-fst)."""
from pathlib import Path
import subprocess
import argparse
import tempfile

root = Path(__file__).resolve().parent.parent
parser = argparse.ArgumentParser(description=__doc__)
parser.add_argument('bindir', type=Path, nargs='?')
parser.add_argument('--reader', type=Path, required=True)
parser.add_argument('--suffix', default='')
args = parser.parse_args()
bindir = args.bindir.resolve() if args.bindir else None
iverilog = 'iverilog' + args.suffix
vvp = 'vvp' + args.suffix
if bindir:
    iverilog, vvp = bindir / iverilog, bindir / vvp
reader = args.reader.resolve()
with tempfile.TemporaryDirectory(prefix='iverilog-fst-enum-') as tmp:
    work = Path(tmp)
    def run(*args):
        return subprocess.check_output([str(a) for a in args], cwd=work, text=True)
    run(iverilog, '-g2012', '-o', work / 'sim.vvp',
        root / 'ivtest/ivltests/fst_enum.v')
    run(vvp, work / 'sim.vvp', '-fst')
    tables, variables, changes = {}, {}, {}
    for line in run(reader, work / 'fst_enum.fst').splitlines():
        kind, a, b, value = line.split(maxsplit=3)
        if kind == 'TABLE':
            tables.setdefault(int(a), {})[b] = value
        elif kind == 'VAR':
            variables[value.split()[0]] = (int(a), int(b))
        else:
            changes.setdefault(int(b), []).append((int(a), value))
    expected = {
        'fst_enum.state': dict(zip(('S_IDLE', 'S_QUEUE_WAIT', 'S_QUEUE_READ',
                           'S_REQ_WAIT', 'S_WRITE', 'S_BRESP'),
                          ('000', '001', '010', '011', '100', '101'))),
        'fst_enum.other': {'OTHER_A': '001', 'OTHER_B': '110'},
        'fst_enum.signed_state': {'NEG': '110', 'POS': '011'},
        'fst_enum.wide': {'WIDE_A': '1' + '0' * 64, 'WIDE_B': '1' + '0' * 63 + '1'},
        'fst_enum.signed_bit': {'BIT_NEG': '111', 'BIT_POS': '010'},
        'fst_enum.integer_state': {'INT_NEG': format((1 << 32) - 3, '032b'), 'INT_POS': format(8, '032b')},
        'fst_enum.four_state': {'UNKNOWN': 'x01', 'HIGH_Z': 'z10'},
    }
    for name, literals in expected.items():
        assert variables[name][1] in tables, f'Missing enum table: {name}'
        assert tables[variables[name][1]] == literals, name
    assert variables['fst_enum.shared'][1] == variables['fst_enum.state'][1]
    assert variables['fst_enum.plain'][1] == 0
    package_literals = {'PKG_IDLE': '00', 'PKG_BUSY': '11'}
    package_ids = []
    for name in ('fst_enum.package_state', 'fst_enum.u0.state', 'fst_enum.u1.state'):
        handle, table = variables[name]
        assert tables[table] == package_literals, name
        assert changes[handle] == [(0, '00'), (10000, '11')], name
        package_ids.append(table)
    assert len(set(package_ids)) == 1, 'Package typedef must share one table across scopes'
    assert tables[variables['fst_enum.anonymous_state'][1]] == {'ANON_A': '01', 'ANON_B': '10'}
    assert changes[variables['fst_enum.anonymous_state'][0]] == [(0, '01'), (10000, '10')]
    assert len(tables) == len(expected) + 2
    assert changes[variables['fst_enum.state'][0]] == [
        (n * 10000, format(n, '03b')) for n in range(6)]
    for name, values in (('fst_enum.signed_state', ['110', '011']),
                         ('fst_enum.wide', list(expected['fst_enum.wide'].values())),
                         ('fst_enum.four_state', ['x01', 'z10'])):
        assert changes[variables[name][0]] == [(0, values[0]), (50000, values[1])], name
    assert changes[variables['fst_enum.plain'][0]] == [(0, '011')]
print('PASS: FST enum tables, type associations, and numeric waveforms')
