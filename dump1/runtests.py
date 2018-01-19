import sys
import getopt
import os
import Image
import subprocess
import tempfile
import numpy as np

def main(dump1, ref_dir, quiet, only = None):
    dump_dir = os.path.join(ref_dir, "dumps")
    alltraces = set([fn.replace(".vc1dump", "") for fn in os.listdir(dump_dir) if fn.endswith(".vc1dump")])

    # This is a list of tests to exclude.
    # Please remove passing tests from this list to prevent regressions
    notyet = set([
        'test_aa_stencil.0',
        'test_alpha_killed.0',
        'test_call.0',
        'test_clear_simple.0',
        'test_dodecahedron.0', 
        'test_edge_polygon.0',
        'test_edge_subpixel.0',
        'test_jump.0',
        'test_kitchensink.0',
        'test_line_extreme.0',
        'test_line_gradients.0',
        'test_line_mini.0',
        'test_line_scatter.0',
        'test_line_scatter.1',
        'test_line_scatter.2',
        'test_line_scatter.3',
        'test_lines_parse.0',
        'test_lines_subpixel.0',
        'test_lines_xy.0',
        'test_line_wide_offscreen.0',
        'test_line_width.0',
        'test_macro.0',
        'test_outside_beginend.0',
        'test_points_increase.0',
        'test_points_large.0',
        'test_points_offscreen.0',
        'test_points_subpixel.0',
        'test_points_visit.0',
        'test_prim_scissor.0',
        'test_rects.0',
        'test_rr_01.0',
        'test_rr_06.0',
        'test_scissor.0',
        'test_scissor_connected.0',
        'test_scissor_max.0',
        'test_scissor_overlap.0',
        'test_shortlines.0',
        'test_state_saverestore.0',
        'test_textvga_1.0',
        'test_uiscape.0',
    ])

    def run1(t):
        expected = Image.open(os.path.join(dump_dir, t + ".png"))
        w,h = expected.size
        bgra = None
        try:
            subprocess.check_call([dump1, os.path.join(dump_dir, t + ".vc1dump"), "out"], stdout = open("0", "w"))
        except subprocess.CalledProcessError:
            return 'CRASH'
        try:
            fi = open("out", mode='rb')
            bgra = Image.fromstring("RGBA", (w, h), fi.read())
            fi.close()
        except ValueError:
            return 'incorrect size %d, expect %d' % (os.path.getsize("out"), w*h*4)
        (b,g,r,a) = bgra.split()
        actual = Image.merge("RGBA", (r,g,b,a))
        error4 = abs(np.array(actual).astype(int) - np.array(expected).astype(int))
        error = 0
        for i in range(4):
            error = np.maximum(error, error4[:,:,i])
        fails = np.argwhere(error > 32)
        if len(fails) > 0:
            expected.save("e.png")
            actual.save("a.png")
            epic = np.where(error > 32, error, 0).astype(np.uint8)
            err = Image.frombuffer("L", expected.size, epic, 'raw', "L", 0, 1).convert("RGBA")

            r = Image.new("RGBA", (3*w, h))
            r.paste(expected, (0, 0))
            r.paste(actual, (w, 0))
            r.paste(err, (2 * w, 0))
            errf = "error_%s.png" % t
            r.convert("RGB").save(errf)
            return "%d pixels differ, differences in %s" % (len(fails), errf)
        else:
            return "pass"

    failed = []
    tests = alltraces - notyet
    if only is not None:
        tests = [only]
    # tests = notyet
    for t in sorted(tests):
        outcome = run1(t)
        if not quiet:
            print "%32s: %s" % (t, outcome)
        if outcome != "pass":
            failed.append(t)
    print "ran %d tests, %d failed" % (len(tests), len(failed))
    if failed:
        print "The following tests failed: " + ", ".join(failed)
    return failed

if __name__ == "__main__":
    try:
        optlist, args = getopt.getopt(sys.argv[1:], "r:d:qo:")
    except getopt.GetoptError:
        print "usage: runtest.py [options]"
        print "  -r <dir>   reference dir (default '../reference')"
        print "  -d exe     path to 'dump1' executable (default 'dump1/dump1')"
        print "  -q         quiet"
        print "  -o <test>  only run <test>"
        sys.exit(1)

    optdict = dict(optlist)

    ref_dir = optdict.get('-r', "../reference")
    dump1 = optdict.get('-d', "dump1/dump1")
    only = optdict.get('-o', None)
    r = main(dump1, ref_dir, quiet = '-q' in optdict, only = only) 
    if not r:
        sys.exit(0)
    else:
        sys.exit(1) # there were failures, so exit nonzero
