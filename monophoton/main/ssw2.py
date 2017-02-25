#!/usr/bin/env python

import sys
import os
import socket
import time
import shutil
import collections
from subprocess import Popen, PIPE
from argparse import ArgumentParser

argParser = ArgumentParser(description = 'Plot and count')
argParser.add_argument('snames', metavar = 'SAMPLE', nargs = '*', help = 'Sample names to skim.')
argParser.add_argument('--list', '-L', action = 'store_true', dest = 'list', help = 'List of samples.')
#argParser.add_argument('--eos-input', '-e', action = 'store_true', dest = 'eosInput', help = 'Specify that input needs to be read from eos.')
argParser.add_argument('--nentries', '-N', metavar = 'N', dest = 'nentries', type = int, default = -1, help = 'Maximum number of entries.')
argParser.add_argument('--compile-only', '-C', action = 'store_true', dest = 'compileOnly', help = 'Compile and exit.')
argParser.add_argument('--json', '-j', metavar = 'PATH', dest = 'json', default = '/cvmfs/cvmfs.cmsaf.mit.edu/hidsk0001/cmsprod/cms/json/Cert_271036-284044_13TeV_23Sep2016ReReco_Collisions16_JSON.txt', help = 'Good lumi list to apply.')
argParser.add_argument('--catalog', '-c', metavar = 'PATH', dest = 'catalog', default = '/home/cmsprod/catalog/t2mit', help = 'Source file catalog.')
argParser.add_argument('--filesets', '-f', metavar = 'ID', dest = 'filesets', nargs = '+', default = [], help = 'Fileset id to run on.')
argParser.add_argument('--suffix', '-x', metavar = 'SUFFIX', dest = 'outSuffix', default = '', help = 'Output file suffix. If running on a single fileset, automatically set to _<fileset>.')
argParser.add_argument('--split', '-B', action = 'store_true', dest = 'split', help = 'Use condor-run to run one instance per fileset. Output is merged at the end.')
argParser.add_argument('--merge-only', '-M', action = 'store_true', dest = 'mergeOnly', help = 'Merge the fragments without running any skim jobs.')

# eosInput:
# Case for running on LXPLUS (used for ICHEP 2016 with simpletree from MINIAOD)
# not maintained any more - use github to recover

args = argParser.parse_args()
sys.argv = []

thisdir = os.path.dirname(os.path.realpath(__file__))
basedir = os.path.dirname(thisdir)
monoxdir = os.path.dirname(basedir)

sys.path.append(monoxdir + '/common')
from goodlumi import makeGoodLumiFilter

sys.path.append(basedir)
import config
from datasets import allsamples
from main.skimconfig import selectors

sys.path.append('/home/yiiyama/lib')
from condor_run import CondorRun

if args.split and len(args.filesets) != 0:
    print 'Split mode must be inclusive in filesets.'
    sys.exit(1)

if len(args.filesets) == 1 and not args.outSuffix:
    args.outSuffix = '_' + args.filesets[0]

import ROOT

ROOT.gSystem.Load(config.libobjs)
ROOT.gSystem.AddIncludePath('-I' + config.dataformats + '/interface')
ROOT.gSystem.AddIncludePath('-I' + os.path.dirname(basedir) + '/common')

ROOT.gROOT.LoadMacro(thisdir + '/Skimmer.cc+')
try:
    s = ROOT.Skimmer
except:
    print "Couldn't compile Skimmer.cc. Quitting."
    sys.exit(1)

if args.compileOnly:
    sys.exit(0)

if 'all' in args.snames:
    spatterns = selectors.keys()
elif 'bkgd' in args.snames:
    spatterns = selectors.keys() + ['!add*', '!dm*']
else:
    spatterns = list(args.snames)

samples = allsamples.getmany(spatterns)

if args.list:
    print ' '.join(sorted(s.name for s in samples))
    sys.exit(0)

filesets = {} # sample -> list of filesets

for sample in samples:
    print 'Starting sample %s (%d/%d)' % (sample.name, samples.index(sample) + 1, len(samples))

    selnames = []
    for rname, gen in selectors[sample.name]:
        selnames.append(rname)

    splitOutDir = config.skimDir + '/' + sample.name

    if len(args.filesets) != 0:
        outDir = splitOutDir
    else:
        outDir = config.skimDir

    if not os.path.exists(outDir):
        os.makedirs(outDir)

    # Read the catalogs and make lists of filesets

    # First get all catalogs (including extensions)
    cnames = [('', sample.fullname)]
    for altname in sample.altnames:
        start = 0
        while True:
            if altname[start] != sample.fullname[start]:
                break
            start += 1
    
        end = -1
        while True:
            if altname[end] != sample.fullname[end]:
                break
            end -= 1
    
        # end cannot be -1 - don't mix data from different tiers!
        dsuffix = altname[start:end + 1]
        if dsuffix.endswith('-v'): # special case if we have e.g. extN suffix but the version number is the same
            dsuffix = dsuffix[:-2]
    
        cnames.append((dsuffix, altname))

    filesets[sample.name] = []
    directories = {} # fileset -> directory of source files
    fullpaths = []

    # Loop over dataset names of the sample
    for dsuffix, dataset in cnames:
        with open(args.catalog + '/' + sample.book + '/' + dataset + '/Filesets') as filesetList:
            for line in filesetList:
                fileset, xrdpath = line.split()[:2]
                fileset += dsuffix

                directories[fileset] = xrdpath.replace('root://xrootd.cmsaf.mit.edu/', '/mnt/hadoop/cms')
                filesets[sample.name].append(fileset)

        with open(args.catalog + '/' + sample.book + '/' + dataset + '/Files') as fileList:
            for line in fileList:
                fileset, fname = line.split()[:2]
                fileset += dsuffix

                if len(args.filesets) == 0 or fileset in args.filesets:
                    fullpaths.append(directories[fileset] + '/' + fname)

    if args.split or args.mergeOnly:
        # Split mode - only need to collect the input names
        # Will spawn condor jobs below
        continue

    # Will do the actual skimming

    skimmer = ROOT.Skimmer()
    skimmer.setCommonSelection('superClusters.rawPt > 175. && TMath::Abs(superClusters.eta) < 1.4442')

    for rname, gen in selectors[sample.name]:
        selector = gen(sample, rname)
        skimmer.addSelector(selector)

    tmpDir = '/tmp/' + os.environ['USER'] + '/' + sample.name
    if not os.path.exists(tmpDir):
        os.makedirs(tmpDir)

    for path in fullpaths:
        if not os.path.exists(path):
            fname = os.path.basename(path)
            dataset = os.path.basename(os.path.dirname(path))
            proc = Popen(['/usr/local/DynamicData/SmartCache/Client/addDownloadRequest.py', '--file', fname, '--dataset', dataset, '--book', sample.book], stdout = PIPE, stderr = PIPE)
            print proc.communicate()[0].strip()

        skimmer.addPath(path)

    if sample.data and args.json:
        skimmer.setGoodLumiFilter(makeGoodLumiFilter(args.json))

    outNameBase = sample.name + '_' + args.outSuffix

    skimmer.run(tmpDir, outNameBase, sample.data, args.nentries)

    for selname in selnames:
        outName = outNameBase + '_' + selname + '.root'

        shutil.copy(tmpDir + '/' + outName, outDir)
        os.remove(tmpDir + '/' + outName)


heldJobs = dict([(sname, []) for sname in filesets.keys()])

if args.split and not args.mergeOnly:
    # Spawn condor jobs
    arguments = []

    # Collect arguments and remove output
    for sname, fslist in filesets.items():
        for fileset in fslist:
            if len(args.filesets) != 0 and fileset not in args.filesets:
                continue

            arguments.append((sname, fileset))

            for selname in [rname for rname, gen in selectors[sname]]:
                path = splitOutDir + '/' + sname + '_' + fileset + '_' + selname + '.root'
                try:
                    os.remove(path)
                except:
                    pass

    submitter = CondorRun(os.path.realpath(__file__))
    submitter.hold_on_fail = True
    submitter.group = 'group_t3mit.urgent'
    submitter.job_args = ['%s -f %s' % arg for arg in arguments]
    submitter.job_names = ['%s_%s' % arg for arg in arguments]

    jobClusters = submitter.submit(name = 'ssw2')

    print 'Waiting for individual skim jobs to complete.'

    while True:
        proc = Popen(['condor_q'] + jobClusters + ['-af', 'ClusterId', 'JobStatus', 'Arguments'], stdout = PIPE, stderr = PIPE)
        out, err = proc.communicate()
        lines = out.split('\n')
        completed = True
        for line in lines:
            if line.strip() == '':
                continue

            clusterId, jobStatus = line.split()[:2]
            if jobStatus == '5':
                sname, dummy, fileset = line.split()[3:] # [2] is the executable
                if fileset in heldJobs[sname]:
                    continue

                print 'Job %s %s is held' % (sname, fileset)
                heldJobs[sname].append(fileset)
                continue
            
            completed = False

        if completed:
            break

        time.sleep(10)

if args.split or args.mergeOnly:    
    print 'Merging the split skims.'

    padd = os.environ['CMSSW_BASE'] + '/bin/' + os.environ['SCRAM_ARCH'] + '/padd'

    for sname, fslist in filesets.items():
        if len(heldJobs[sname]) != 0:
            print 'Some jobs failed for ' + sname + '. Not merging output.'
            continue

        for selname in [rname for rname, gen in selectors[sname]]:
            outName = sname + '_' + selname + '.root'

            proc = Popen([padd, '/tmp/' + outName] + [splitOutDir + '/' + sname + '_' + fileset + '_' + selname + '.root' for fileset in fslist], stdout = PIPE, stderr = PIPE)
            out, err = proc.communicate()
            print out.strip()
            print err.strip()
    
            shutil.copy('/tmp/' + outName, config.skimDir)
            os.remove('/tmp/' + outName)
