int m_victim[TFMaxPlayers];
bool m_isCoverBlown[TFMaxPlayers];

public bool SpyHuntRequirements(int client)
{
	if (!TF2_IsPlayerInCondition(client, TFCond_Disguised))
		return false;
	
	if (TF2_HasTheFlag(client))
		return false;
	
	return true;
}

public void SpyHuntStart(int client)
{
	m_isCoverBlown[client] = false;
	DeletePathNodes(client);
}

public void SpyHuntUpdate(int client)
{
	FindFriendsAndEnemiens(client);
	FindEnemyEntities(client);

	if (m_useTeleporter[client] && IsValidEntity(m_teleporterEntity[client]))
	{
		DeletePathNodes(client);
		m_nextStuckCheck[client] = GetGameTime() + 5.0;
		if (GetVectorDistance(GetOrigin(client), GetOrigin(m_teleporterEntity[client]), true) <= Squaredf(24.0))
		{
			m_moveVel[client][0] = 0.0;
			m_moveVel[client][1] = 0.0;
			m_moveVel[client][2] = 0.0;
		}
		else
			MoveToWaypoint(client, GetOrigin(m_teleporterEntity[client]));
		return;
	}

	if (!DeactiveCloak(client))
	{
		LookAround(client);
		if (IsValidClient(m_goalEntity[client]) || IsValidEntity(m_goalEntity[client]))
			FollowPath(client, GetOrigin(m_goalEntity[client]));

		// avoid bumping!!!
		if (m_hasEnemiesNear[client] && IsValidClient(m_nearestEnemy[client]) && GetVectorDistance(GetOrigin(client), GetOrigin(m_nearestEnemy[client]), true) <= Squaredf(200.0))
			BackstabMove(client, m_nearestEnemy[client], false);

		return;
	}

	CheckHealth(client);
	CheckAmmo(client);

	int threat = m_goalEntity[client];
	int closestThreat = m_nearestEnemy[client];
	
	// opportunistically attack closer threat if they are much closer to us than our existing threat
	if (!IsValidClient(threat) || !IsPlayerAlive(threat))
	{
		threat = closestThreat;
		m_isCoverBlown[client] = false;
		if (IsValidClient(closestThreat) && IsPlayerAlive(closestThreat))
			m_victim[client] = closestThreat;
	}
	else if (IsValidClient(closestThreat) && IsPlayerAlive(closestThreat) && closestThreat != threat)
	{
		float rangeToCurrentThreat = GetVectorDistance(GetOrigin(client), GetOrigin(threat), true);
		float rangeToNewThreat = GetVectorDistance(GetOrigin(client), GetOrigin(closestThreat), true);

		if (rangeToCurrentThreat - rangeToNewThreat > Squaredf(FindConVar("tf_bot_spy_change_target_range_threshold").FloatValue))
		{
			threat = closestThreat;
			m_victim[client] = closestThreat;
			m_isCoverBlown[client] = false;
		}
	}
	
	SpyAimLogic(client);

	if (!IsValidClient(threat) || !IsPlayerAlive(threat))
	{
		FinishCurrentProcess(client, false, "No threat");
		return;
	}

	int sapTarget = m_knownSentry[client];
	if (IsValidEntity(sapTarget) && GetVectorDistance(GetOrigin(client), GetOrigin(sapTarget), true) < GetVectorDistance(GetOrigin(client), GetOrigin(closestThreat), true))
	{
		SetProcess(client, PRO_SPYSAP, 60.0, "Opportunistically sapping an enemy object between my victim and I");
		return;
	}

	if (IsValidEntity(sapTarget) && IsVisible(GetEyePosition(client), GetCenter(sapTarget)))// me->IsAnyEnemySentryAbleToAttackMe() )
	{
		m_isCoverBlown[client] = true;
		EquipWeaponSlot(client, 0);
		ActiveCloak(client);
		return;
	}

	int playerThreat = threat;
	if (!IsValidClient(threat) || !IsPlayerAlive(threat))
	{
		FinishCurrentProcess(client, false, "Current 'threat' is not a player or a building?");
		return;
	}

	// remember who we are attacking (in case we changed our minds)
	m_victim[client] = playerThreat;

	bool isKnifeFight = false;

	if (TF2_IsPlayerInCondition(client, TFCond_Disguised) || TF2_IsPlayerInCondition(client, TFCond_Disguising))
		isKnifeFight = true;

	float playerThreatForward[3];
	EyeVectors(playerThreat, playerThreatForward);

	float playerThreatOrigin[3];
	float myOrigin[3];
	playerThreatOrigin = GetOrigin(playerThreat);
	myOrigin = GetOrigin(client);

	float toPlayerThreat[3];
	toPlayerThreat[0] = playerThreatOrigin[0] - myOrigin[0];
	toPlayerThreat[1] = playerThreatOrigin[1] - myOrigin[1];
	toPlayerThreat[2] = playerThreatOrigin[2] - myOrigin[2];

	float threatRange = GetVectorLength(toPlayerThreat, true);

	float behindTolerance = 0.0;

	switch (m_difficulty[client])
	{
		case 1:
			behindTolerance = 0.6;
		case 2:
			behindTolerance = 0.4;
		case 3:
			behindTolerance = 0.2;
		case 4:
			behindTolerance = 0.0;
	}

	bool isBehindVictim = GetVectorDotProduct(playerThreatForward, toPlayerThreat) > behindTolerance;

	// easiest Spies always think they're in position to backstab
	if (m_difficulty[client] == 0)
		isBehindVictim = true;

	if (threatRange <= Squaredf(FindConVar("tf_bot_spy_knife_range").FloatValue))
		isKnifeFight = true;
	else if (IsClientVisibleInScreen(client, playerThreat) && isBehindVictim) // they are facing away from us - go for the backstab
		isKnifeFight = true;

	// does my threat know I'm a Spy?
	if (IsClientAimingToMe(client, playerThreat) && IsAttacking(playerThreat) && GotDamageByLast(client, 4.0))
		m_isCoverBlown[client] = true;
	else if (TF2_IsPlayerInCondition(playerThreat, TFCond_Disguised) || TF2_IsPlayerInCondition(playerThreat, TFCond_Cloaked))
		m_isCoverBlown[client] = true;
	
	if (m_isCoverBlown[client] || 
	TF2_IsPlayerInCondition(client, TFCond_OnFire) ||
	TF2_IsPlayerInCondition(client, TFCond_Jarated) || 
	TF2_IsPlayerInCondition(client, TFCond_CloakFlicker) || 
	TF2_IsPlayerInCondition(client, TFCond_Bleeding) || 
	TF2_IsPlayerInCondition(client, TFCond_Milked) || 
	TF2_IsPlayerInCondition(playerThreat, TFCond_Charging) || 
	TF2_IsPlayerInCondition(playerThreat, TFCond_Ubercharged))
		isKnifeFight = false;
	
	EquipWeaponSlot(client, isKnifeFight ? 2 : 0);
	bool isMovingTowardVictim = true;

	if (isKnifeFight && IsWeaponSlotActive(client, 2))
	{
		const float circleStrafeRange = 200.0;

		if (threatRange <= Squaredf(circleStrafeRange) && IsVisible(GetEyePosition(client), GetEyePosition(playerThreat)))
		{
			// we're close - aim our stab attack
			m_lookAt[client] = GetEyePosition(playerThreat);
			isMovingTowardVictim = false;

			if (!isBehindVictim)
			{
				// circle around our victim to get behind them
				float flBotAng[3], flTargetAng[3];
				GetClientEyeAngles(client, flBotAng);
				GetClientEyeAngles(playerThreat, flTargetAng);
				float iAngleDiff = AngleDiff(flBotAng[1], flTargetAng[1]);
	
				if (iAngleDiff > 90)
					m_moveVel[client][1] = -300.0;
				else if (iAngleDiff < -90)
					m_moveVel[client][1] = 300.0;
			}

			// don't continue to close in if we're already very close so we don't bump them and give ourselves away
			if (threatRange <= Squaredf(72.0)) 
				m_moveVel[client][0] = -300.0;
			else if (threatRange >= Squaredf(84.0))
				m_moveVel[client][0] = 300.0;
		}

		if (threatRange <= Squaredf(84.0))
		{
			// if we're still disguised, go for the backstab
			if (TF2_IsPlayerInCondition(client, TFCond_Disguised))
			{
				// we're behind them (or they're onto us) - backstab!
				if ((m_difficulty[client] != 4 && isBehindVictim) || GetClientHealth(playerThreat) <= 40)
					m_buttons[client] |= IN_ATTACK;
				else if (m_isCoverBlown[client])
				{
					if (!m_lowHealth[client] && !m_lowAmmo[client] && m_class[playerThreat] != TFClass_Heavy)
						m_buttons[client] |= IN_ATTACK;
					else
						ActiveCloak(client);
				}
			}
			else
			{
				// we're exposed - stab! stab! stab!
				if (!m_lowHealth[client] && !m_lowAmmo[client] && GetClientHealth(playerThreat) < 100.0 && m_class[playerThreat] != TFClass_Heavy)
					m_buttons[client] |= IN_ATTACK;
				else
					ActiveCloak(client);
			}
		}
	}
	else if (!isKnifeFight && IsWeaponSlotActive(client, 0))
		m_attackTimer[client] = GetGameTime() + 0.20;

	if (isMovingTowardVictim)
	{
		// pursue the threat. if not visible, go to the last known position
		if (!m_hasEnemiesNear[client] && threatRange > FindConVar("tf_bot_spy_knife_range").FloatValue)
		{
			// if we're at the threat's last known position and he's still not visible, we lost him
			if (!IsVisible(GetEyePosition(client), GetEyePosition(playerThreat)))
			{
				if (GetVectorDistance(GetOrigin(client), m_lastEnemyOrigin[client], true) <= Squaredf(100.0))
				{
					if (m_goalEntity[client] == threat)
						m_goalEntity[client] = -1;

					FinishCurrentProcess(client, false, "I lost my target!");
					return;
				}
			}
			else
				m_lastEnemyOrigin[client] = GetOrigin(playerThreat);
		}

		FollowPath(client, GetOrigin(playerThreat));
	}
	else
		DeletePathNodes(client);

	// avoid bumping!!!
	if (m_hasEnemiesNear[client] && IsValidClient(m_nearestEnemy[client]) && GetVectorDistance(GetOrigin(client), GetOrigin(m_nearestEnemy[client]), true) <= Squaredf(200.0))
		BackstabMove(client, m_nearestEnemy[client], false);
}

public void SpyHuntEnd(int client)
{
	if (IsValidClient(m_goalEntity[client]) && ChanceOf(m_eBotSenseChance[client]))
		FakeClientCommandThrottled(client, "lastdisguise");
}

public void SpyReactChecker(int client) 
{
	if (IsWeaponSlotActive(client, 5) && !m_hasEnemiesNear[client] && !m_hasEntitiesNear[client])
		EquipWeaponSlot(client, 2);
}

public void SpyAimLogic(int client)
{
	if (TF2_IsPlayerInCondition(client, TFCond_Disguised) && m_hasEnemiesNear[client] && GetClientAimTarget(m_nearestEnemy[client]) == client && IsAttacking(m_nearestEnemy[client])) // enemy looking at me
	{
		LookAtEnemiens(client);
		return;
	}
	
	int sqDist = Squared(300);
	if (TF2_IsPlayerInCondition(client, TFCond_Disguised) && m_hasEnemiesNear[client] && m_enemyDistance[client] > sqDist && GetClientAimTarget(m_nearestEnemy[client]) == client) // don't look at enemy
	{
		if (m_hasFriendsNear[client])
			m_lookAt[client] = GetEyePosition(m_hasFriendsNear[client]);
		else
			LookAround(client);
		return;
	}
	else if (TF2_IsPlayerInCondition(client, TFCond_Disguised) && m_hasEnemiesNear[client] && !m_hasFriendsNear[client])
	{
		if (m_enemyDistance[client] <= sqDist)
			LookAtEnemiens(client);
		else
			LookAround(client);
		return;
	}

	if (m_hasEnemiesNear[client])
		LookAtEnemiens(client);
	else
		LookAround(client);
}
