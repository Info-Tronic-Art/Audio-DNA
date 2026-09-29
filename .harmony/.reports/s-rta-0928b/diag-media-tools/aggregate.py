# aggregate.py PREFIX [PREFIX...] -- per arm (run-tag prefix), per phase: the median across launches of each metric,
# with the per-launch values listed (>= 5 launches per arm for a ranked number). Reads runs/<tag>/analysis.json.
import glob, json, os, sys, statistics as st
from collections import defaultdict

SP = '/private/tmp/claude-501/-Users-boriskarpman-projects-RealTimeAudio/e9ff9dc6-f159-4d37-b0a9-871bf522d258/scratchpad/diag-media'


def get(d, path):
    for k in path.split('.'):
        if d is None: return None
        d = d.get(k) if isinstance(d, dict) else None
    return d


METRICS = [
    ('fps', 'steady.gl.fps'), ('cb_med', 'steady.gl.cb.med'), ('cb_p90', 'steady.gl.cb.p90'), ('cb_max', 'steady.gl.cb.max'),
    ('iv_p90', 'steady.gl.interval.p90'), ('iv_max', 'steady.gl.interval.max'),
    ('iv>20/5s', 'steady.gl.iv_gt_20'), ('iv>34/5s', 'steady.gl.iv_gt_34'), ('cb>8', 'steady.gl.cb_gt_8'),
    ('cb>16', 'steady.gl.cb_gt_16'), ('nodeck_all', 'all.gl.nodeck'),
    ('vid_adv_dec_med', 'steady.video.adv_decoding.med'), ('vid_adv_dec_p90', 'steady.video.adv_decoding.p90'),
    ('vid_adv_dec_max', 'steady.video.adv_decoding.max'), ('vid_adv_all_med', 'steady.video.adv_all.med'),
    ('dec_each_med', 'steady.video.dec_ms_each.med'), ('conv_med', 'steady.video.conv_ms.med'),
    ('flip_med', 'steady.video.flip_ms.med'), ('upl_med', 'steady.video.upl_ms.med'), ('upl_p90', 'steady.video.upl_ms.p90'),
    ('upl_calls', 'steady.video.upl_calls'), ('dec_calls', 'steady.video.decoding_calls'),
    ('ndec_max', 'steady.video.ndec_per_call.max'), ('seeks', 'steady.video.seeks'), ('ndec>=5', 'steady.video.calls_ndec_ge5'),
    ('ex_n_med', 'steady.gl.exists_per_frame_n.med'), ('ex_us_med', 'steady.gl.exists_us_each.med'),
    ('ex_us_p90', 'steady.gl.exists_us_each.p90'), ('ex_ms_per_s', 'steady.gl.exists_ms_per_s'),
    ('seq_ms_med', 'steady.gl.seq_ms.med'), ('seq_ms_max', 'steady.gl.seq_ms.max'), ('seq_upl_n', 'steady.gl.seq_upl_n'),
    ('seq_upl_med', 'steady.gl.seq_uplms_each.med'), ('seq_upl_max', 'steady.gl.seq_uplms_each.max'),
    ('hb_max_all', 'all.msg.hb_max'), ('hb>16_all', 'all.msg.hb_gt_16'), ('hb>50_all', 'all.msg.hb_gt_50'),
    ('hb_med_steady', 'steady.msg.hb_lat.med'), ('hb_p90_steady', 'steady.msg.hb_lat.p90'),
    ('hb_sum_ms/s_steady', 'steady.msg.hb_sum_ms_per_s'),
    ('paint/s', 'all.paint.per_s'), ('paint_ms_med', 'all.paint.ms.med'), ('paint_ms_p90', 'all.paint.ms.p90'),
    ('stats/s', 'all.paint.stats_per_s'), ('stat_us_med', 'all.paint.stat_us_each.med'),
    ('stat_us_p90', 'all.paint.stat_us_each.p90'), ('stat_us_max', 'all.paint.stat_us_each.max'),
    ('stat_ms/s', 'all.paint.stat_ms_per_s'), ('paint_ms/s', 'all.paint.paint_ms_per_s'),
]
SCOPES = ['mc.loadComposition', 'mc.openMediaForDeck', 'omd.videoClip', 'vp.open.total', 'vp.open.firstFrame',
          'vp.getThumbnail', 'omd.seqClip', 'seq.open.total', 'seq.open.statLoop', 'seq.open.frame0Decode',
          'omd.seqThumbDecode', 'mc.swapCompositionModel', 'us.fence.wait', 'us.fence.mutation', 'hook.total',
          'mc.applyFileDrop', 'mc.applyMultiFileDrop', 'amfd.thumbDecode', 'mc.handleClipTrigger', 'dv.refresh',
          'rd.getVideoPlayer.lockwait', 'rd.closeMedia.video', 'rd.openVideo.insertLock']


def med(v):
    v = [x for x in v if x is not None]
    return None if not v else round(st.median(v), 3)


def main():
    out = {}
    for prefix in sys.argv[1:]:
        runs = sorted(d for d in glob.glob(SP + '/runs/' + prefix + '[0-9]*') if os.path.exists(d + '/analysis.json')
                      and not (os.path.exists(d + '/compiler-samples.txt') and os.path.getsize(d + '/compiler-samples.txt') > 0))
        per = defaultdict(lambda: defaultdict(list))
        loads = []
        for r in runs:
            a = json.load(open(r + '/analysis.json'))
            try:
                loads.append(open(r + '/meta.txt').read().split('load: ')[1].split('}')[0] + '}')
            except Exception:  # noqa: BLE001
                pass
            for ph in a['phases']:
                for name, path in METRICS:
                    per[ph['name']][name].append(get(ph, path))
                for sc in SCOPES:
                    s = (ph['all'].get('scopes') or {}).get(sc)
                    if s:
                        per[ph['name']]['S:' + sc + ':n'].append(s['n'])
                        per[ph['name']]['S:' + sc + ':med'].append(s['med'])
                        per[ph['name']]['S:' + sc + ':max'].append(s['max'])
                        per[ph['name']]['S:' + sc + ':sum'].append(round(s['mean'] * s['n'], 3))
        arm = {'runs': [os.path.basename(r) for r in runs], 'loads_at_start': loads, 'phases': {}}
        for phn, mets in per.items():
            arm['phases'][phn] = {k: {'median': med(v), 'per_launch': v} for k, v in mets.items() if any(x is not None for x in v)}
        out[prefix] = arm
    json.dump(out, open(SP + '/runs/aggregate-' + '-'.join(sys.argv[1:]) + '.json', 'w'), indent=1)
    for prefix, arm in out.items():
        print('==', prefix, len(arm['runs']), 'launches', arm['runs'])
        for phn, mets in arm['phases'].items():
            keys = [k for k in mets if not k.startswith('S:')]
            print('  %-22s ' % phn + ' '.join('%s=%s' % (k, mets[k]['median']) for k in keys))
            sk = [k for k in mets if k.startswith('S:')]
            if sk:
                print('  %-22s ' % '' + ' '.join('%s=%s' % (k[2:], mets[k]['median']) for k in sk))


main()
