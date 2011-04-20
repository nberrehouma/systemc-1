/*****************************************************************************

  The following code is derived, directly or indirectly, from the SystemC
  source code Copyright (c) 1996-2011 by all Contributors.
  All Rights reserved.

  The contents of this file are subject to the restrictions and limitations
  set forth in the SystemC Open Source License Version 3.0 (the "License");
  You may not use this file except in compliance with such restrictions and
  limitations. You may obtain instructions on how to receive a copy of the
  License at http://www.systemc.org/. Software distributed by Contributors
  under the License is distributed on an "AS IS" basis, WITHOUT WARRANTY OF
  ANY KIND, either express or implied. See the License for the specific
  language governing rights and limitations under the License.

 *****************************************************************************/

/*****************************************************************************

  sc_method_process.cpp -- Method process implementation

  Original Author: Andy Goodrich, Forte Design Systems, 4 August 2005
               

 *****************************************************************************/

/*****************************************************************************

  MODIFICATION LOG - modifiers, enter your name, affiliation, date and
  changes you are making here.

      Name, Affiliation, Date:
  Description of Modification:

 *****************************************************************************/

// $Log: sc_method_process.cpp,v $
// Revision 1.41  2011/04/19 19:15:41  acg
//  Andy Goodrich: fix so warning message is always issued for a throw_it()
//  on a method process.
//
// Revision 1.40  2011/04/19 15:04:27  acg
//  Philipp A. Hartman: clean up SC_ID messages.
//
// Revision 1.39  2011/04/19 02:39:09  acg
//  Philipp A. Hartmann: added checks for additional throws during stack unwinds.
//
// Revision 1.38  2011/04/13 02:41:34  acg
//  Andy Goodrich: eliminate warning messages generated when the DEBUG_MSG
//  macro is used.
//
// Revision 1.37  2011/04/11 22:10:46  acg
//  Andy Goodrich:
//    (1) Add DEBUG_MSG macro and use it to generate a journal of method
//        throws if it is enabled.
//    (2) Trim down to the expected behavior of scheduling a method that
//        is asynchronously reset in anticipation of IEEE 1666 being revised.
//
// Revision 1.36  2011/04/10 22:15:29  acg
//  Andy Goodrich: change to call methods on asynchronous reset.
//
// Revision 1.35  2011/04/08 22:31:40  acg
//  Andy Goodrich: removed unused code.
//
// Revision 1.34  2011/04/08 18:24:07  acg
//  Andy Goodrich: fix asynchronous reset dispatch and when the reset_event()
//  is fired.
//
// Revision 1.33  2011/04/05 20:50:56  acg
//  Andy Goodrich:
//    (1) changes to make sure that event(), posedge() and negedge() only
//        return true if the clock has not moved.
//    (2) fixes for method self-resumes.
//    (3) added SC_PRERELEASE_VERSION
//    (4) removed kernel events from the object hierarchy, added
//        sc_hierarchy_name_exists().
//
// Revision 1.32  2011/04/01 22:30:39  acg
//  Andy Goodrich: change hard assertion to warning for trigger_dynamic()
//  getting called when there is only STATIC sensitivity. This can result
//  because of sc_process_handle::throw_it().
//
// Revision 1.31  2011/03/28 13:02:51  acg
//  Andy Goodrich: Changes for disable() interactions.
//
// Revision 1.30  2011/03/23 16:17:52  acg
//  Andy Goodrich: don't emit an error message for a resume on a disabled
//  process that is not suspended.
//
// Revision 1.29  2011/03/20 13:43:23  acg
//  Andy Goodrich: added async_signal_is() plus suspend() as a corner case.
//
// Revision 1.28  2011/03/08 20:49:30  acg
//  Andy Goodrich: implement coarse checking for synchronous reset - suspend
//  interaction.
//
// Revision 1.27  2011/03/08 20:32:28  acg
//  Andy Goodrich: implemented "coarse" checking for undefined process
//  control interactions.
//
// Revision 1.26  2011/03/07 18:25:19  acg
//  Andy Goodrich: tightening of check for resume on a disabled process to
//  only produce an error if it is ready to run.
//
// Revision 1.25  2011/03/07 17:38:43  acg
//  Andy Goodrich: tightening up of checks for undefined interaction between
//  synchronous reset and suspend.
//
// Revision 1.24  2011/03/06 23:30:13  acg
//  Andy Goodrich: refining suspend - sync reset corner case checking so that
//  the following are error situations:
//    (1) Calling suspend on a process with a reset_signal_is() specification
//        or sync_reset_on() is active.
//    (2) Calling sync_reset_on() on a suspended process.
//
// Revision 1.23  2011/03/06 19:57:11  acg
//  Andy Goodrich: refinements for the illegal suspend - synchronous reset
//  interaction.
//
// Revision 1.22  2011/03/06 16:47:09  acg
//  Andy Goodrich: changes for testing sync_reset - suspend corner cases.
//
// Revision 1.21  2011/03/06 15:57:08  acg
//  Andy Goodrich: added process control corner case checks.
//
// Revision 1.20  2011/03/05 19:44:20  acg
//  Andy Goodrich: changes for object and event naming and structures.
//
// Revision 1.19  2011/02/18 20:27:14  acg
//  Andy Goodrich: Updated Copyrights.
//
// Revision 1.18  2011/02/17 19:50:43  acg
//  Andy Goodrich:
//    (1) Changed signature of trigger_dynamic back to a bool.
//    (2) Added run queue processing into trigger dynamic.
//    (3) Simplified process control support.
//
// Revision 1.17  2011/02/16 22:37:30  acg
//  Andy Goodrich: clean up to remove need for ps_disable_pending.
//
// Revision 1.16  2011/02/13 23:09:58  acg
//  Andy Goodrich: only remove dynamic events for asynchronous resets.
//
// Revision 1.15  2011/02/13 21:47:37  acg
//  Andy Goodrich: update copyright notice.
//
// Revision 1.14  2011/02/13 21:31:02  acg
//  Andy Goodrich: added error messages for throws when simulator has not
//  been initialized. Added missing remove_dynamic_events() call to the
//  reset code.
//
// Revision 1.13  2011/02/11 13:25:24  acg
//  Andy Goodrich: Philipp A. Hartmann's changes:
//    (1) Removal of SC_CTHREAD method overloads.
//    (2) New exception processing code.
//
// Revision 1.12  2011/02/07 19:17:20  acg
//  Andy Goodrich: changes for IEEE 1666 compatibility.
//
// Revision 1.11  2011/02/04 15:27:36  acg
//  Andy Goodrich: changes for suspend-resume semantics.
//
// Revision 1.10  2011/02/01 23:01:53  acg
//  Andy Goodrich: removed dead code.
//
// Revision 1.9  2011/02/01 21:05:05  acg
//  Andy Goodrich: Changes in trigger_dynamic methods to handle new
//  process control rules about event sensitivity.
//
// Revision 1.8  2011/01/25 20:50:37  acg
//  Andy Goodrich: changes for IEEE 1666 2011.
//
// Revision 1.7  2011/01/18 20:10:44  acg
//  Andy Goodrich: changes for IEEE1666_2011 semantics.
//
// Revision 1.6  2011/01/06 18:02:43  acg
//  Andy Goodrich: added check for ps_disabled to method_dynamic().
//
// Revision 1.5  2010/11/20 17:10:56  acg
//  Andy Goodrich: reset processing changes for new IEEE 1666 standard.
//
// Revision 1.4  2010/07/22 20:02:33  acg
//  Andy Goodrich: bug fixes.
//
// Revision 1.3  2009/05/22 16:06:29  acg
//  Andy Goodrich: process control updates.
//
// Revision 1.2  2008/05/22 17:06:25  acg
//  Andy Goodrich: updated copyright notice to include 2008.
//
// Revision 1.1.1.1  2006/12/15 20:20:05  acg
// SystemC 2.3
//
// Revision 1.7  2006/04/20 17:08:16  acg
//  Andy Goodrich: 3.0 style process changes.
//
// Revision 1.6  2006/04/11 23:13:20  acg
//   Andy Goodrich: Changes for reduced reset support that only includes
//   sc_cthread, but has preliminary hooks for expanding to method and thread
//   processes also.
//
// Revision 1.5  2006/01/26 21:04:54  acg
//  Andy Goodrich: deprecation message changes and additional messages.
//
// Revision 1.4  2006/01/24 20:49:05  acg
// Andy Goodrich: changes to remove the use of deprecated features within the
// simulator, and to issue warning messages when deprecated features are used.
//
// Revision 1.3  2006/01/13 18:44:29  acg
// Added $Log to record CVS changes into the source.
//

#include "sysc/kernel/sc_method_process.h"
#include "sysc/kernel/sc_simcontext_int.h"
#include "sysc/kernel/sc_module.h"
#include "sysc/kernel/sc_spawn_options.h"

// DEBUGGING MACROS:
//
// DEBUG_MSG(NAME,P,MSG)
//     MSG  = message to print
//     NAME = name that must match the process for the message to print, or
//            null if the message should be printed unconditionally.
//     P    = pointer to process message is for, or NULL in which case the
//            message will not print.
#if 0
#   define DEBUG_NAME ""
#   define DEBUG_MSG(NAME,P,MSG) \
    { \
        if ( P && ( (strlen(NAME)==0) || !strcmp(NAME,P->name())) ) \
          std::cout << sc_time_stamp() << ": " << P->name() << " ******** " \
                    << MSG << std::endl; \
    }
#else
#   define DEBUG_MSG(NAME,P,MSG) 
#endif

namespace sc_core {

//------------------------------------------------------------------------------
//"sc_method_process::clear_trigger"
//
// This method clears any pending trigger for this object instance.
//------------------------------------------------------------------------------
void sc_method_process::clear_trigger()
{
    switch( m_trigger_type ) {
      case STATIC: 
        return;
      case EVENT: 
        m_event_p->remove_dynamic( this );
        m_event_p = 0;
        break;
      case OR_LIST: 
        m_event_list_p->remove_dynamic( this, 0 );
        m_event_list_p->auto_delete();
        m_event_list_p = 0;
        break;
      case AND_LIST: 
        m_event_list_p->remove_dynamic( this, 0 );
        m_event_list_p->auto_delete();
        m_event_list_p = 0;
        m_event_count = 0;
        break;
      case TIMEOUT: 
        m_timeout_event_p->cancel();
        m_timeout_event_p->reset();
        break;
      case EVENT_TIMEOUT: 
        m_timeout_event_p->cancel();
        m_timeout_event_p->reset();
        m_event_p->remove_dynamic( this );
        m_event_p = 0;
        break;
      case OR_LIST_TIMEOUT: 
        m_timeout_event_p->cancel();
        m_timeout_event_p->reset();
        m_event_list_p->remove_dynamic( this, 0 );
        m_event_list_p->auto_delete();
        m_event_list_p = 0;
      break;
      case AND_LIST_TIMEOUT: 
        m_timeout_event_p->cancel();
        m_timeout_event_p->reset();
        m_event_list_p->remove_dynamic( this, 0 );
        m_event_list_p->auto_delete();
        m_event_list_p = 0;
        m_event_count = 0;
        break;
    }
    m_trigger_type = STATIC;
}

//------------------------------------------------------------------------------
//"sc_method_process::disable_process"
//
// This virtual method disables this process and its children if requested to.
//     descendants = indicator of whether this process' children should also
//                   be suspended
//------------------------------------------------------------------------------
void sc_method_process::disable_process(
    sc_descendant_inclusion_info descendants )
{     
    int                              child_i;    // Index of child accessing.
    int                              child_n;    // Number of children.
    sc_process_b*                    child_p;    // Child accessing.
    const ::std::vector<sc_object*>* children_p; // Vector of children.

    // IF NEEDED PROPOGATE THE SUSPEND REQUEST THROUGH OUR DESCENDANTS:

    if ( descendants == SC_INCLUDE_DESCENDANTS )
    {
        children_p = &get_child_objects();
        child_n = children_p->size();
        for ( child_i = 0; child_i < child_n; child_i++ )
        {
            child_p = DCAST<sc_process_b*>((*children_p)[child_i]);
            if ( child_p ) child_p->disable_process(descendants);
        }
    }

    // DON'T ALLOW CORNER CASE BY DEFAULT:

    if ( !sc_allow_process_control_corners )
    {
	switch( m_trigger_type )
	{ 
	  case AND_LIST_TIMEOUT:
	  case EVENT_TIMEOUT: 
	  case OR_LIST_TIMEOUT:
	  case TIMEOUT:
	    SC_REPORT_ERROR( SC_ID_PROCESS_CONTROL_CORNER_CASE_,
		": attempt to disable a method with timeout wait" );
	    break;
	  default:
	    break;
	}
    }

    // DISABLE OUR OBJECT INSTANCE:

    m_state = m_state | ps_bit_disabled;

    // IF THIS CALL IS BEFORE THE SIMULATION DON'T RUN THE METHOD:

    if ( !sc_is_running() )
    {
        sc_get_curr_simcontext()->remove_runnable_method(this);
    }
}


//------------------------------------------------------------------------------
//"sc_method_process::enable_process"
//
// This method enables the execution of this process, and if requested, its
// descendants. If the process was suspended and has a resumption pending it 
// will be dispatched in the next delta cycle. Otherwise the state will be
// adjusted to indicate it is no longer suspended, but no immediate execution
// will occur.
//------------------------------------------------------------------------------
void sc_method_process::enable_process(
    sc_descendant_inclusion_info descendants )
{
    int                              child_i;    // Index of child accessing.
    int                              child_n;    // Number of children.
    sc_process_b*                    child_p;    // Child accessing.
    const ::std::vector<sc_object*>* children_p; // Vector of children.

    // IF NEEDED PROPOGATE THE RESUME REQUEST THROUGH OUR DESCENDANTS:

    if ( descendants == SC_INCLUDE_DESCENDANTS )
    {
        children_p = &get_child_objects();
        child_n = children_p->size();
        for ( child_i = 0; child_i < child_n; child_i++ )
        {
            child_p = DCAST<sc_process_b*>((*children_p)[child_i]);
            if ( child_p ) child_p->enable_process(descendants);
        }
    }

    // ENABLE THIS OBJECT INSTANCE:
    //
    // If it was disabled and ready to run then put it on the run queue.

    m_state = m_state & ~ps_bit_disabled;
    if ( m_state == ps_bit_ready_to_run )
    {
        m_state = ps_normal;
	if ( next_runnable() == 0 )
	    simcontext()->push_runnable_method(this);
    }
}


//------------------------------------------------------------------------------
//"sc_method_process::kill_process"
//
// This method removes throws a kill for this object instance. It calls the
// sc_process_b::kill_process() method to perform low level clean up. 
//------------------------------------------------------------------------------
void sc_method_process::kill_process(sc_descendant_inclusion_info descendants)
{
    int                              child_i;    // Index of child accessing.
    int                              child_n;    // Number of children.
    sc_process_b*                    child_p;    // Child accessing.
    const ::std::vector<sc_object*>* children_p; // Vector of children.

    // IF THE SIMULATION HAS NOT BEEN INITIALIZED YET THAT IS AN ERROR:

    if ( sc_get_status() == SC_ELABORATION )
    {
        SC_REPORT_ERROR( SC_ID_KILL_PROCESS_WHILE_UNITIALIZED_, "" );
    }

    // IF THE PROCESS IS CURRENTLY UNWINDING THAT IS AN ERROR:

    if ( m_unwinding )
    {
        SC_REPORT_ERROR( SC_ID_PROCESS_ALREADY_UNWINDING_, name() );
    }

    // IF NEEDED, PROPOGATE THE KILL REQUEST THROUGH OUR DESCENDANTS:

    if ( descendants == SC_INCLUDE_DESCENDANTS )
    {
        children_p = &get_child_objects();
        child_n = children_p->size();
        for ( child_i = 0; child_i < child_n; child_i++ )
        {
            child_p = DCAST<sc_process_b*>((*children_p)[child_i]);
            if ( child_p ) child_p->kill_process(descendants);
        }
    }

    // REMOVE OUR PROCESS FROM EVENTS, ETC., AND QUEUE IT SO IT CAN 
    // THROW ITS KILL.

    disconnect_process();
    if ( next_runnable() != 0 )
        simcontext()->remove_runnable_method( this );

    if ( RCAST<sc_method_handle>(sc_get_current_process_b()) == this )
    {
        m_throw_status = THROW_KILL;
        throw sc_unwind_exception( this, false );
    }
}

//------------------------------------------------------------------------------
//"sc_method_process::sc_method_process"
//
// This is the object instance constructor for this class.
//------------------------------------------------------------------------------
sc_method_process::sc_method_process( const char* name_p, 
    bool free_host, SC_ENTRY_FUNC method_p, 
    sc_process_host* host_p, const sc_spawn_options* opt_p 
):
    sc_process_b(
        name_p ? name_p : sc_gen_unique_name("method_p"), 
        false, free_host, method_p, host_p, opt_p)
{

    // CHECK IF THIS IS AN sc_module-BASED PROCESS AND SIMUALTION HAS STARTED:

    if ( DCAST<sc_module*>(host_p) != 0 && sc_is_running() )
    {
        SC_REPORT_ERROR( SC_ID_MODULE_METHOD_AFTER_START_, "" );
    }

    // INITIALIZE VALUES:
    //
    // If there are spawn options use them.

    m_process_kind = SC_METHOD_PROC_;
    if (opt_p) {
        m_dont_init = opt_p->m_dont_initialize;

        // traverse event sensitivity list
        for (unsigned int i = 0; i < opt_p->m_sensitive_events.size(); i++) {
            sc_sensitive::make_static_sensitivity(
                this, *opt_p->m_sensitive_events[i]);
        }

        // traverse port base sensitivity list
        for ( unsigned int i = 0; i < opt_p->m_sensitive_port_bases.size(); i++)
        {
            sc_sensitive::make_static_sensitivity(
                this, *opt_p->m_sensitive_port_bases[i]);
        }

        // traverse interface sensitivity list
        for ( unsigned int i = 0; i < opt_p->m_sensitive_interfaces.size(); i++)
        {
            sc_sensitive::make_static_sensitivity(
                this, *opt_p->m_sensitive_interfaces[i]);
        }

        // traverse event finder sensitivity list
        for ( unsigned int i = 0; i < opt_p->m_sensitive_event_finders.size();
            i++)
        {
            sc_sensitive::make_static_sensitivity(
                this, *opt_p->m_sensitive_event_finders[i]);
        }

	// process any reset signal specification:

	opt_p->specify_resets();
    }

    else
    {
        m_dont_init = false;
    }
}

//------------------------------------------------------------------------------
//"sc_method_process::sc_method_process"
//
// This is the object instance destructor for this class.
//------------------------------------------------------------------------------
sc_method_process::~sc_method_process()
{
}


//------------------------------------------------------------------------------
//"sc_method_process::suspend_process"
//
// This virtual method suspends this process and its children if requested to.
//     descendants = indicator of whether this process' children should also
//                   be suspended
//------------------------------------------------------------------------------
void sc_method_process::suspend_process(
    sc_descendant_inclusion_info descendants )
{     
    int                              child_i;    // Index of child accessing.
    int                              child_n;    // Number of children.
    sc_process_b*                    child_p;    // Child accessing.
    const ::std::vector<sc_object*>* children_p; // Vector of children.

    // IF NEEDED PROPOGATE THE SUSPEND REQUEST THROUGH OUR DESCENDANTS:

    if ( descendants == SC_INCLUDE_DESCENDANTS )
    {
        children_p = &get_child_objects();
        child_n = children_p->size();
        for ( child_i = 0; child_i < child_n; child_i++ )
        {
            child_p = DCAST<sc_process_b*>((*children_p)[child_i]);
            if ( child_p ) child_p->suspend_process(descendants);
        }
    }

    // CORNER CASE CHECKS, THE FOLLOWING ARE ERRORS:
    //   (a) if this method has a reset_signal_is specification 
    //   (b) if this method is in synchronous reset

    if ( !sc_allow_process_control_corners && m_has_reset_signal )
    {
	SC_REPORT_ERROR(SC_ID_PROCESS_CONTROL_CORNER_CASE_,
		    ": attempt to suspend a method that has a reset signal");
    }
    else if ( !sc_allow_process_control_corners && m_sticky_reset )
    {
	SC_REPORT_ERROR(SC_ID_PROCESS_CONTROL_CORNER_CASE_,
		    ": attempt to suspend a method in synchronous reset");
    }

    // SUSPEND OUR OBJECT INSTANCE:
    //
    // (1) If we are on the runnable queue then set suspended and ready_to_run,
    //     and remove ourselves from the run queue.
    // (2) If this is a self-suspension then a resume should cause immediate
    //     scheduling of the process.

    m_state = m_state | ps_bit_suspended;
    if ( next_runnable() != 0 ) 
    {
	m_state = m_state | ps_bit_ready_to_run;
	simcontext()->remove_runnable_method( this );
    }
    if ( sc_get_current_process_b() == DCAST<sc_process_b*>(this)  )
    {
	m_state = m_state | ps_bit_ready_to_run;
    }
}

//------------------------------------------------------------------------------
//"sc_method_process::resume_process"
//
// This method resumes the execution of this process, and if requested, its
// descendants. If the process was suspended and has a resumption pending it 
// will be dispatched in the next delta cycle. Otherwise the state will be
// adjusted to indicate it is no longer suspended, but no immediate execution
// will occur.
//------------------------------------------------------------------------------
void sc_method_process::resume_process(
    sc_descendant_inclusion_info descendants )
{
    int                              child_i;    // Index of child accessing.
    int                              child_n;    // Number of children.
    sc_process_b*                    child_p;    // Child accessing.
    const ::std::vector<sc_object*>* children_p; // Vector of children.

    // IF NEEDED PROPOGATE THE RESUME REQUEST THROUGH OUR DESCENDANTS:

    if ( descendants == SC_INCLUDE_DESCENDANTS )
    {
        children_p = &get_child_objects();
        child_n = children_p->size();
        for ( child_i = 0; child_i < child_n; child_i++ )
        {
            child_p = DCAST<sc_process_b*>((*children_p)[child_i]);
            if ( child_p ) child_p->resume_process(descendants);
        }
    }


    // BY DEFAULT THE CORNER CASE IS AN ERROR:

    if ( !sc_allow_process_control_corners && (m_state & ps_bit_disabled) &&
         (m_state & ps_bit_suspended) )
    {
	m_state = m_state & ~ps_bit_suspended;
        SC_REPORT_ERROR(SC_ID_PROCESS_CONTROL_CORNER_CASE_, 
	               ": call to resume() on a disabled suspended method");
    }

    // CLEAR THE SUSPENDED BIT:

    m_state = m_state & ~ps_bit_suspended;

    // RESUME OBJECT INSTANCE:
    //
    // If this is not a self-resume and the method is ready to run then
    // put it on the runnable queue.

    if ( m_state & ps_bit_ready_to_run )
    {
	m_state = m_state & ~ps_bit_ready_to_run;
	if ( next_runnable() == 0 && 
	   ( sc_get_current_process_b() != DCAST<sc_process_b*>(this) ) )
        {
	    simcontext()->push_runnable_method(this);
	    remove_dynamic_events();  
	}
    }
}

//------------------------------------------------------------------------------
//"sc_method_process::throw_reset"
//
// This virtual method is invoked to "throw" a reset. 
//
// If the reset is synchronous this is a no-op, except for triggering the
// reset event if it is present.
//
// If the reset is asynchronous we:
//   (a) cancel any dynamic waits 
//   (b) if it is the active process actually throw a reset exception.
//   (c) if it was not the active process and does not have a static
//       sensitivity emit an error if corner cases are to be considered
//       errors.
//
// Notes:
//   (1) If the process had a reset event it will have been triggered in 
//       sc_process_b::semantics()
//
// Arguments:
//   async = true if this is an asynchronous reset.
//------------------------------------------------------------------------------
void sc_method_process::throw_reset( bool async )
{
    // If the process is currently unwinding this is an error:

    if ( m_unwinding )
    {
        SC_REPORT_ERROR( SC_ID_PROCESS_ALREADY_UNWINDING_, name() );
    }

    // Set the throw status and if its an asynchronous reset throw an
    // exception:

    m_throw_status = async ? THROW_ASYNC_RESET : THROW_SYNC_RESET;
    if ( async )
    {
        remove_dynamic_events();
	if ( RCAST<sc_method_handle>(sc_get_current_process_b()) == this )
	{
	    DEBUG_MSG(DEBUG_NAME,this,"throw_reset: throwing exception");
	    m_throw_status = THROW_ASYNC_RESET;
	    throw sc_unwind_exception( this, true );
	}
	else 
	{
	    DEBUG_MSG(DEBUG_NAME,this,
	              "throw_reset: queueing method for execution");
	    simcontext()->execute_method_next(this);
	}
    }
}


//------------------------------------------------------------------------------
//"sc_method_process::throw_user"
//
// This virtual method is invoked when a user exception is to be thrown.
// If requested it will also throw the exception to the children of this 
// object instance. Since this is a method no throw will occur for this
// object instance. The children will be awakened from youngest child to
// eldest.
//     helper_p    -> object to use to throw the exception.
//     descendants =  indicator of whether this process' children should also
//                    be suspended
//------------------------------------------------------------------------------
void sc_method_process::throw_user( const sc_throw_it_helper& helper,
    sc_descendant_inclusion_info descendants )
{     
    int                              child_i;    // Index of child accessing.
    int                              child_n;    // Number of children.
    sc_process_b*                    child_p;    // Child accessing.
    const ::std::vector<sc_object*>* children_p; // Vector of children.

    // IF THE SIMULATION IS NOT ACTUALLY RUNNING THIS IS AN ERROR:

    if (  sc_get_status() != SC_RUNNING )
    {
        SC_REPORT_ERROR( SC_ID_THROW_IT_WHILE_NOT_RUNNING_, name() );
    }

    // IF NEEDED PROPOGATE THE THROW REQUEST THROUGH OUR DESCENDANTS:

    if ( descendants == SC_INCLUDE_DESCENDANTS )
    {
        children_p = &get_child_objects();
        child_n = children_p->size();
        for ( child_i = 0; child_i < child_n; child_i++ )
        {
            child_p = DCAST<sc_process_b*>((*children_p)[child_i]);
            if ( child_p ) child_p->throw_user(helper, descendants);
        }
    }

    // throw_it HAS NO EFFECT ON A METHOD, ISSUE A WARNING:

    SC_REPORT_WARNING( SC_ID_THROW_IT_ON_METHOD_, name() );

}

//------------------------------------------------------------------------------
//"sc_method_process::trigger_dynamic"
//
// This method sets up a dynamic trigger on an event.
//   dt_rearm      - don't execute the method and don't remove it from the
//                   event's queue.
//   dt_remove     - the thread should not be scheduled for execution and the
//                   process should be removed from the event's method queue.
//   dt_run        - the thread should be scheduled for execution but the
//                   the proces should stay on the event's thread queue.
//   dt_run_remove - the thread should be scheduled for execution and the
//                   process should be removed from the event's thread queue.
//
// Notes:
//   (1) This method is identical to sc_thread_process::trigger_dynamic(), 
//       but they cannot be combined as sc_process_b::trigger_dynamic() 
//       because the signatures things like sc_event::remove_dynamic()
//       have different overloads for sc_method_process* and sc_thread_process*.
//       So if you change code here you'll also need to change it in 
//       sc_thread_process.cpp.
// Result is true if this process should be removed from the event's list,
// false if not.
//------------------------------------------------------------------------------
bool sc_method_process::trigger_dynamic( sc_event* e )
{
    // No time outs yet, and keep gcc happy.

    m_timed_out = false;

    // Escape cases:
    //   (a) If this method issued the notify() don't schedule it for execution,
    //       but leave the sensitivity in place.
    //   (b) If this method is already runnable can't trigger an event.

    if ( sc_get_current_process_b() == (sc_process_b*)this )
        return false;
    else if( is_runnable() ) 
        return true;

    // If a process is disabled then we ignore any events, leaving them enabled:
    //
    // But if this is a time out event we need to remove both it and the
    // event that was being waited for.

    if ( m_state & ps_bit_disabled )
    {
        if ( e == m_timeout_event_p )
	{
	    remove_dynamic_events( true );  
	    return true;
	}
	else
	{
	    return false;
	}
    }


    // Process based on the event type and current process state:
    //
    // Every case needs to set 'rc' and continue on to the end of
    // this method to allow suspend processing to work correctly.

    switch( m_trigger_type ) 
    {
      case EVENT: 
	m_event_p = 0;
	m_trigger_type = STATIC;
	break;

      case AND_LIST:
        -- m_event_count;
	if ( m_event_count == 0 )
	{
	    m_event_list_p->auto_delete();
	    m_event_list_p = 0;
	    m_trigger_type = STATIC;
	}
	else
	{
	    return true;
	}
	break;

      case OR_LIST:
	m_event_list_p->remove_dynamic( this, e );
	m_event_list_p->auto_delete();
	m_event_list_p = 0;
	m_trigger_type = STATIC;
	break;

      case TIMEOUT: 
	m_trigger_type = STATIC;
	break;

      case EVENT_TIMEOUT: 
        if ( e == m_timeout_event_p )
	{
	    m_timed_out = true;
	    m_event_p->remove_dynamic( this );
	    m_event_p = 0;
	    m_trigger_type = STATIC;
	}
	else
	{
	    m_timeout_event_p->cancel();
	    m_timeout_event_p->reset();
	    m_event_p = 0;
	    m_trigger_type = STATIC;
	}
	break;

      case OR_LIST_TIMEOUT:
        if ( e == m_timeout_event_p )
	{
            m_timed_out = true;
            m_event_list_p->remove_dynamic( this, e ); 
            m_event_list_p->auto_delete();
            m_event_list_p = 0; 
            m_trigger_type = STATIC;
	}

	else
	{
            m_timeout_event_p->cancel();
            m_timeout_event_p->reset();
	    m_event_list_p->remove_dynamic( this, e ); 
	    m_event_list_p->auto_delete();
	    m_event_list_p = 0; 
	    m_trigger_type = STATIC;
	}
	break;
      
      case AND_LIST_TIMEOUT:
        if ( e == m_timeout_event_p )
	{
            m_timed_out = true;
            m_event_list_p->remove_dynamic( this, e ); 
            m_event_list_p->auto_delete();
            m_event_list_p = 0; 
            m_trigger_type = STATIC;
	}

	else
	{
	    -- m_event_count;
	    if ( m_event_count == 0 )
	    {
		m_timeout_event_p->cancel();
		m_timeout_event_p->reset();
		// no need to remove_dynamic
		m_event_list_p->auto_delete();
		m_event_list_p = 0; 
		m_trigger_type = STATIC;
	    }
	    else
	    {
	        return true;
	    }
	}
	break;

      case STATIC: {
        // we should never get here, but throw_it() can make it happen.
	SC_REPORT_WARNING(SC_ID_NOT_EXPECTING_DYNAMIC_EVENT_NOTIFY_, name());
        return true;
      }
    }

    // If we get here then the method has satisfied its next_trigger, if its 
    // suspended mark its state as ready to run. If its not suspended then push
    // it onto the runnable queue.

    if ( (m_state & ps_bit_suspended) )
    {
	m_state = m_state | ps_bit_ready_to_run;
    }
    else
    {
        simcontext()->push_runnable_method(this);
    }

    return true;
}

} // namespace sc_core 
