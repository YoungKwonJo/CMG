
id99Failed = 'jetsVLId99Failed.@obj.size()>0'
id95Failed = 'jetsVLId95Failed.@obj.size()>0'
beamHaloCSCLooseFailed = 'beamHaloCSCLoose==1'
beamHaloCSCTightFailed = 'beamHaloCSCTight==1'
hbheNoise2010Failed = 'hbheNoise2010.obj==0'
hbheNoise2011IsoFailed = 'hbheNoise2011Iso.obj==0'
hbheNoise2011NonIsoFailed = 'hbheNoise2011NonIso.obj==0'
ecalDeadTPFailed = 'ecalDeadTP==0'
muVetoFailed = 'mus.@obj.size()>0'
eleVetoFailed = 'eles.@obj.size()>0'

id95_30_Failed = 'jetsPBNR_GNH95_30.@obj.size()>0'
id95_50_Failed = 'jetsPBNR_GNH95_50.@obj.size()>0'
idNH95Failed =   'jetsPBNR_NH95.@obj.size()>0'
idNH90Failed =   'jetsPBNR_NH90.@obj.size()>0'
idNH80Failed =   'jetsPBNR_NH80.@obj.size()>0'
idG95Failed =    'jetsPBNR_G95.@obj.size()>0'

idNH90G95Failed = '(jetsPBNR_NH90.@obj.size()>0 || jetsPBNR_G95.@obj.size()>0)'
idNH90G95Passed = '!( (jetsPBNR_NH90.@obj.size()>0 || jetsPBNR_G95.@obj.size()>0) )'

dPhiFailed = '!(ra2dphi0.obj>0.5 && ra2dphi1.obj>0.5 && ra2dphi2.obj>0.3)'
dPhiPassed = 'ra2dphi0.obj>0.5 && ra2dphi1.obj>0.5 && ra2dphi2.obj>0.3'
dPhi0Failed = 'ra2dphi0.obj<0.5'
dPhi1Failed = 'ra2dphi1.obj<0.5'
dPhi2Failed = 'ra2dphi2.obj<0.3'