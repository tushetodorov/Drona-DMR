#include <iostream>
#include <stdlib.h>
#include "TestDriver.h"
#include "Workspace.h"
#include "PlanGenerator.h"
#include "Compat.h"

PRT_VALUE *P_FUN_Sleep_IMPL(PRT_MACHINEINST *context) {
	PRT_MACHINEINST_PRIV *p_tmp_mach_priv = (PRT_MACHINEINST_PRIV *)context;
	PRT_VALUE *p_tmp_ret = NULL;
	PRT_FUNSTACK_INFO p_tmp_frame;
	PRT_VALUE *p_tmp_params;
	int time = 0;
	p_tmp_params = NULL;
	//remm to pop frame
	PrtPopFrame(p_tmp_mach_priv, &p_tmp_frame);
	//create a tuple value
	time = PrtPrimGetInt(p_tmp_frame.locals[0]);
	SleepMs(time);
	//remm to free the frame
	PrtFreeLocals(p_tmp_mach_priv, &p_tmp_frame);
	return p_tmp_ret;
}

PRT_VALUE *P_FUN_GetNumOfRobots_IMPL(PRT_MACHINEINST *context) {
	PRT_MACHINEINST_PRIV *p_tmp_mach_priv = (PRT_MACHINEINST_PRIV *)context;
	PRT_VALUE *p_tmp_ret = NULL;
	PRT_FUNSTACK_INFO p_tmp_frame;
	PRT_VALUE *p_tmp_params;
	int time = 0;
	p_tmp_params = NULL;
	//remm to pop frame
	PrtPopFrame(p_tmp_mach_priv, &p_tmp_frame);
	PrtFreeLocals(p_tmp_mach_priv, &p_tmp_frame);
	return PrtMkIntValue(WORKSPACE_INFO->numOfRobots);
}

PRT_VALUE *P_FUN_GetRobotInfo_IMPL(PRT_MACHINEINST *context) {
	PRT_MACHINEINST_PRIV *p_tmp_mach_priv = (PRT_MACHINEINST_PRIV *)context;
	PRT_VALUE *p_tmp_ret = NULL;
	PRT_FUNSTACK_INFO p_tmp_frame;
	PRT_VALUE *p_tmp_params;
	PRT_VALUE* retVal;
	int robotId;
	retVal = PrtMkDefaultValue(P_GEND_TYPE_RobotInfoType);
	p_tmp_params = NULL;
	//remm to pop frame
	PrtPopFrame(p_tmp_mach_priv, &p_tmp_frame);
	robotId = PrtPrimGetInt(p_tmp_frame.locals[0]);
	PrtFreeLocals(p_tmp_mach_priv, &p_tmp_frame);

	PrtTupleSetEx(retVal, 0, PrtMkIntValue(WORKSPACE_INFO->robots[robotId].id), PRT_FALSE);
	PrtTupleSetEx(retVal, 1, PrtMkIntValue(WORKSPACE_INFO->robots[robotId].start_position), PRT_FALSE);
	return retVal;
}

PRT_VALUE *P_FUN_GetGoalLocations_IMPL(PRT_MACHINEINST *context) {
	PRT_VALUE* retSeq;
	PRT_TYPE* intType;
	PRT_TYPE* intSeqType;
	int counter = 0;
	PRT_MACHINEINST_PRIV *p_tmp_mach_priv = (PRT_MACHINEINST_PRIV *)context;
	PRT_VALUE *p_tmp_ret = NULL;
	PRT_FUNSTACK_INFO p_tmp_frame;
	PRT_VALUE *p_tmp_params;
	int time = 0;
	p_tmp_params = NULL;
	//remm to pop frame
	PrtPopFrame(p_tmp_mach_priv, &p_tmp_frame);
	PrtFreeLocals(p_tmp_mach_priv, &p_tmp_frame);

	intType = PrtMkPrimitiveType(PRT_KIND_INT);
	intSeqType = PrtMkSeqType(intType);
	retSeq = PrtMkDefaultValue(intSeqType);
	PrtFreeType(intType); PrtFreeType(intSeqType);
	
	for (counter = 0; counter < WORKSPACE_INFO->starts.size; counter++)
	{
		PrtSeqInsertExIntIndex(retSeq, counter, PrtMkIntValue(WORKSPACE_INFO->starts.locations[counter]), PRT_FALSE);
	}
	for (counter = 0; counter < WORKSPACE_INFO->ends.size; counter++)
	{
		PrtSeqInsertExIntIndex(retSeq, counter, PrtMkIntValue(WORKSPACE_INFO->ends.locations[counter]), PRT_FALSE);
	}
	return retSeq;
}

/*
model fun PlanGenerator(s: int, g: int, avoids: seq[seq[int]]) : seq[int] {
var ret : seq[int];
ret += (0, 1);
return ret;
}
*/
PRT_VALUE *P_FUN_DistributedMotionPlannerMachine_PlanGenerator_IMPL(PRT_MACHINEINST *context) {
	PRT_TYPE* intType;
	PRT_TYPE* intSeqType;
	int counter = 0;
	PRT_MACHINEINST_PRIV *p_tmp_mach_priv = (PRT_MACHINEINST_PRIV *)context;
	PRT_VALUE *p_tmp_ret = NULL;
	PRT_FUNSTACK_INFO p_tmp_frame;
	PRT_VALUE *p_tmp_params;
	int time = 0;
	p_tmp_params = NULL;
	//remm to pop frame
	PrtPopFrame(p_tmp_mach_priv, &p_tmp_frame);
	int startLocation = PrtPrimGetInt(p_tmp_frame.locals[0]);
	int goalLocation = PrtPrimGetInt(p_tmp_frame.locals[1]);
	PRT_VALUE* avoidsList = p_tmp_frame.locals[2];
	int robotid = PrtPrimGetInt(p_tmp_frame.locals[3]);
	printf("%d starts calculation for goalLocation %d\n", robotid, goalLocation);
	int sizeOfAvoids = PrtSeqSizeOf(avoidsList);
	
	int* output_seq_of_locations = (int*)malloc(1000 * sizeof(int));
	int output_size = 0;
	vector< vector<WS_Coord> > avoidsArr(sizeOfAvoids);
	for (int i = 0; i < sizeOfAvoids; i++)
	{
		avoidsArr[i] = vector<WS_Coord>();
		PRT_VALUE* avoid_p = PrtSeqGetNCIntIndex(avoidsList, i);
		for (int j = 0; j < PrtSeqSizeOf(avoid_p); j++)
		{
			avoidsArr[i].push_back(ExtractCoordFromGridLocation(PrtPrimGetInt(PrtSeqGetNCIntIndex(avoid_p, j)), WORKSPACE_INFO->dimension));
		}
	}

	bool success;
	PRT_VALUE* retPlan;
	intType = PrtMkPrimitiveType(PRT_KIND_INT);
	intSeqType = PrtMkSeqType(intType);
	retPlan = PrtMkDefaultValue(intSeqType);
	PrtFreeType(intType); PrtFreeType(intSeqType);

	success = GenerateMotionPlanFor(robotid, *WORKSPACE_INFO, startLocation, goalLocation, WORKSPACE_INFO->obstacles.locations, WORKSPACE_INFO->obstacles.size, avoidsArr, output_seq_of_locations, &output_size);
	
	if(success) {	
		for (int counter = 0; counter < output_size; counter++)
		{
			PrtSeqInsertExIntIndex(retPlan, counter, PrtMkIntValue(output_seq_of_locations[counter]), PRT_FALSE);
		}
	}

	PrtFreeLocals(p_tmp_mach_priv, &p_tmp_frame);
	return retPlan;
}

#if defined(PRT_PLAT_WINUSER) || defined(USE_EMPTY)
PRT_VALUE *P_FUN_StartExecutingPath_IMPL(PRT_MACHINEINST *context) {
	PRT_MACHINEINST_PRIV *p_tmp_mach_priv = (PRT_MACHINEINST_PRIV *)context;
	PRT_VALUE *p_tmp_ret = NULL;
	PRT_FUNSTACK_INFO p_tmp_frame;
	PRT_VALUE *p_tmp_params;
	int time = 0;
	p_tmp_params = NULL;
	//remm to pop frame
	PrtPopFrame(p_tmp_mach_priv, &p_tmp_frame);
	PrtFreeLocals(p_tmp_mach_priv, &p_tmp_frame);
	return NULL;
}

PRT_VALUE *P_FUN_RosInit_IMPL(PRT_MACHINEINST *context) {
	PRT_MACHINEINST_PRIV *p_tmp_mach_priv = (PRT_MACHINEINST_PRIV *)context;
	PRT_VALUE *p_tmp_ret = NULL;
	PRT_FUNSTACK_INFO p_tmp_frame;
	PRT_VALUE *p_tmp_params;
	int time = 0;
	p_tmp_params = NULL;
	//remm to pop frame
	PrtPopFrame(p_tmp_mach_priv, &p_tmp_frame);
	PrtFreeLocals(p_tmp_mach_priv, &p_tmp_frame);
	return NULL;
}
#endif

static void PrintStat() {
	const char* out_stat_file_name = getenv("TEST_DRIVER_OUT_STAT_FILE");

    std::ostream* out = &cout;
    std::ofstream fout;
    
	if(out_stat_file_name) {
        fout.open(out_stat_file_name, std::ofstream::out | std::ofstream::app);
        out = &fout;
	}

	double total_elapsed_secs = double(total_elapsed_clock) / CLOCKS_PER_SEC;
	double total_elapsed_secs_ext = double(total_elapsed_clock_ext) / CLOCKS_PER_SEC;
	*out << "Total A* solving(timeout included): " << total_elapsed_secs << " seconds" << std::endl;
	*out << "Total A* solving(timeout excluded): " << total_elapsed_secs_ext << " seconds" << std::endl;
	*out << "Total number of A* solver calls(timeout included): " << total_num_calculations << std::endl;
	*out << "Total number of A* solver calls(timeout excluded): " << total_num_calculations_ext << std::endl;
	*out << "Total length of solution(timeout excluded): " << total_length_of_path << std::endl;

	if(out_stat_file_name) {
		fout.close();
	}
}

PRT_VALUE *P_FUN_Main_ExitP_IMPL(PRT_MACHINEINST *context) {
	PRT_MACHINEINST_PRIV *p_tmp_mach_priv = (PRT_MACHINEINST_PRIV *)context;
	PRT_VALUE *p_tmp_ret = NULL;
	PRT_FUNSTACK_INFO p_tmp_frame;
	PRT_VALUE *p_tmp_params;
	int time = 0;
	p_tmp_params = NULL;
	//remm to pop frame
	PrtPopFrame(p_tmp_mach_priv, &p_tmp_frame);
	PrtFreeLocals(p_tmp_mach_priv, &p_tmp_frame);
	PrintStat();
	printf("Done !! Good Bye\n");
	exit(0);
	return NULL;
}

PRT_INT32 G_taskId;
PRT_RECURSIVE_MUTEX t_lock = PrtCreateMutex();

PRT_VALUE *P_FUN_GetUniqueTaskId_IMPL(PRT_MACHINEINST *context) {
	PRT_MACHINEINST_PRIV *p_tmp_mach_priv = (PRT_MACHINEINST_PRIV *)context;
	PRT_VALUE *p_tmp_ret = NULL;
	PRT_FUNSTACK_INFO p_tmp_frame;
	PRT_VALUE *p_tmp_params;
	int t_id = 0;
	p_tmp_params = NULL;
	//remm to pop frame
	PrtPopFrame(p_tmp_mach_priv, &p_tmp_frame);
#ifdef USE_ROBOTID_AS_PRIORITY
	t_id = PrtPrimGetInt(p_tmp_frame.locals[0]);
#else
	PrtLockMutex(t_lock);
	t_id = G_taskId;
	G_taskId++;
	PrtUnlockMutex(t_lock);
#endif
	PrtFreeLocals(p_tmp_mach_priv, &p_tmp_frame);
	return PrtMkIntValue(t_id);
}

PRT_VALUE *P_FUN_DistributedMotionPlannerMachine_GetRandomNumber_IMPL(PRT_MACHINEINST *context) {
	PRT_MACHINEINST_PRIV *p_tmp_mach_priv = (PRT_MACHINEINST_PRIV *)context;
	PRT_VALUE *p_tmp_ret = NULL;
	PRT_FUNSTACK_INFO p_tmp_frame;
	PRT_VALUE *p_tmp_params;
	int t_id = 0;
	p_tmp_params = NULL;
	//remm to pop frame
	PrtPopFrame(p_tmp_mach_priv, &p_tmp_frame);
	t_id = PrtPrimGetInt(p_tmp_frame.locals[0]);
	PrtFreeLocals(p_tmp_mach_priv, &p_tmp_frame);
	
	return PrtMkIntValue((rand() % (int)(t_id)));
}