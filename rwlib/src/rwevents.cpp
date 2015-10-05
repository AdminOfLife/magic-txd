#include "StdInc.h"

#include "pluginutil.hxx"

#include <map>

namespace rw
{

// We want to hold a list of all event handlers in any RwObject.
struct eventSystemManager
{
    struct objectEvents
    {
        struct eventEntry
        {
            struct handler_t
            {
                EventHandler_t cb;
                void *ud;

                inline bool operator ==( EventHandler_t handler )
                {
                    return ( this->cb == handler );
                }
            };

            typedef std::list <handler_t> handlers_t;

            handlers_t eventHandlers;
        };

        inline void Initialize( GenericRTTI *rtObj )
        {
            RwObject *rwObj = (RwObject*)RwTypeSystem::GetObjectFromTypeStruct( rtObj );

            // We start out with an empty map of events.
            this->evtLock = CreateReadWriteLock( rwObj->engineInterface );
        }

        inline void Shutdown( GenericRTTI *rtObj )
        {
            RwObject *rwObj = (RwObject*)RwTypeSystem::GetObjectFromTypeStruct( rtObj );

            // Memory is cleared automatically.
            if ( rwlock *lock = this->evtLock )
            {
                CloseReadWriteLock( rwObj->engineInterface, lock );
            }
        }

        inline void operator = ( const objectEvents& right )
        {
            // Assigning object event handlers makes no sense.
        }

        inline void RegisterEventHandler( event_t eventID, EventHandler_t handler, void *ud )
        {
            scoped_rwlock_writer <rwlock> lock( this->evtLock );

            eventEntry& info = events[ eventID ];

            // Only register if not already registered.
            if ( std::find( info.eventHandlers.begin(), info.eventHandlers.end(), handler ) == info.eventHandlers.end() )
            {
                eventEntry::handler_t item;
                item.cb = handler;
                item.ud = ud;

                info.eventHandlers.push_back( item );
            }
        }

        inline void UnregisterEventHandler( event_t eventID, EventHandler_t handler )
        {
            scoped_rwlock_writer <rwlock> lock( this->evtLock );

            eventMap_t::iterator eventEntryIter = events.find( eventID );

            if ( eventEntryIter == events.end() )
                return; // not found.

            bool shouldRemoveEventEntryIter = false;
            {
                eventEntry& evtEntry = eventEntryIter->second;

                eventEntry::handlers_t::const_iterator iter =
                    std::find( evtEntry.eventHandlers.begin(), evtEntry.eventHandlers.end(), handler );

                if ( iter != evtEntry.eventHandlers.end() )
                {
                    evtEntry.eventHandlers.erase( iter );

                    if ( evtEntry.eventHandlers.empty() )
                    {
                        // Remove this entry.
                        shouldRemoveEventEntryIter = true;
                    }
                }
            }

            if ( shouldRemoveEventEntryIter )
            {
                events.erase( eventEntryIter );
            }
        }

        inline bool TriggerEvent( RwObject *obj, event_t eventID, void *ud )
        {
            scoped_rwlock_reader <rwlock> lock( this->evtLock );

            eventMap_t::iterator eventEntryIter = events.find( eventID );

            if ( eventEntryIter == events.end() )
                return false; // not found.

            eventEntry& evtEntry = eventEntryIter->second;

            bool wasHandled = false;

            for ( eventEntry::handlers_t::const_iterator iter = evtEntry.eventHandlers.begin(); iter != evtEntry.eventHandlers.end(); iter++ )
            {
                // Call it.
                const eventEntry::handler_t& curHandler = *iter;

                curHandler.cb( obj, eventID, ud, curHandler.ud );

                wasHandled = true;
            }

            return wasHandled;
        }

        typedef std::map <event_t, eventEntry> eventMap_t;

        eventMap_t events;

        rwlock *evtLock;
    };

    inline void Initialize( EngineInterface *engineInterface )
    {
        this->pluginOffset =
            engineInterface->typeSystem.RegisterDependantStructPlugin <objectEvents> ( engineInterface->rwobjTypeInfo, RwTypeSystem::ANONYMOUS_PLUGIN_ID );
    }

    inline void Shutdown( EngineInterface *engineInterface )
    {
        engineInterface->typeSystem.UnregisterPlugin( engineInterface->rwobjTypeInfo, this->pluginOffset );
    }

    inline objectEvents* GetPluginStruct( EngineInterface *engineInterface, RwObject *obj )
    {
        GenericRTTI *rtObj = RwTypeSystem::GetTypeStructFromObject( obj );

        return RwTypeSystem::RESOLVE_STRUCT <objectEvents> ( engineInterface, rtObj, engineInterface->rwobjTypeInfo, this->pluginOffset );
    }

    RwTypeSystem::pluginOffset_t pluginOffset;
};

static PluginDependantStructRegister <eventSystemManager, RwInterfaceFactory_t> eventSysRegister;

void RegisterEventHandler( RwObject *obj, event_t eventID, EventHandler_t theHandler, void *ud )
{
    EngineInterface *engineInterface = (EngineInterface*)obj->engineInterface;

    if ( eventSystemManager *eventSys = eventSysRegister.GetPluginStruct( engineInterface ) )
    {
        // Put the event handler into the holder plugin.
        if ( eventSystemManager::objectEvents *objReg = eventSys->GetPluginStruct( engineInterface, obj ) )
        {
            objReg->RegisterEventHandler( eventID, theHandler, ud );
        }
    }
}

void UnregisterEventHandler( RwObject *obj, event_t eventID, EventHandler_t theHandler )
{
    EngineInterface *engineInterface = (EngineInterface*)obj->engineInterface;

    if ( eventSystemManager *eventSys = eventSysRegister.GetPluginStruct( engineInterface ) )
    {
        // Put the event handler into the holder plugin.
        if ( eventSystemManager::objectEvents *objReg = eventSys->GetPluginStruct( engineInterface, obj ) )
        {
            objReg->UnregisterEventHandler( eventID, theHandler );
        }
    }
}

bool TriggerEvent( RwObject *obj, event_t eventID, void *ud )
{
    if ( !AcquireObject( obj ) )
    {
        throw RwException( "failed to get object reference count for event trigger" );
    }

    EngineInterface *engineInterface = (EngineInterface*)obj->engineInterface;

    bool wasHandled = false;

    // Trigger all event handlers that count for this eventID.
    if ( eventSystemManager *eventSys = eventSysRegister.GetPluginStruct( engineInterface ) )
    {
        if ( eventSystemManager::objectEvents *objReg = eventSys->GetPluginStruct( engineInterface, obj ) )
        {
            // TODO: when we create object hierarchies, we want to trigger
            // events for the parents aswell!

            wasHandled = objReg->TriggerEvent( obj, eventID, ud );
        }
    }

    // Release the object reference.
    // This may destroy the object, so be careful!
    ReleaseObject( obj );

    return wasHandled;
}

void registerEventSystem( void )
{
    eventSysRegister.RegisterPlugin( engineFactory );
}

};