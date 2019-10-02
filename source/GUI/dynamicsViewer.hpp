/*
 * dynamicsViewer.hpp
 * Hummus - spiking neural network simulator
 *
 * Created by Omar Oubari.
 * Email: omar.oubari@inserm.fr
 * Last Version: 17/01/2018
 *
 * Information: The DynamicsViewer class is used by the Display class to show a specified neuron's potential and current. Depends on Qt5
 */

#pragma once

#include <algorithm>
#include <atomic>

// QT5 and QT Charts Dependency
#include <QtCore/QObject>

#include <QtQuick/QQuickView>
#include <QtQuick/QQuickItem>
#include <QtQuick/QtQuick>

#include <QtCharts/QAbstractSeries>
#include <QtCharts/QValueAxis>
#include <QtCharts/QAreaSeries>
#include <QtCharts/QXYSeries>
#include <QtCharts/QChart>

namespace hummus {
    
    class DynamicsViewer : public QObject {
        
    Q_OBJECT
    public:
		
        // ----- CONSTRUCTOR AND DESTRUCTOR
        DynamicsViewer(QObject *parent = 0) :
                QObject(parent),
                time_window(100),
                openGL(true),
                is_closed(false),
                max_x(0),
                min_y(-70),
                max_y(-50),
                min_y_right(0),
                max_y_right(1),
                current_plot(false),
                neuron_tracker(-1) {
            atomic_guard.clear(std::memory_order_release);
        }
        
        virtual ~DynamicsViewer(){}
		
    	// ----- PUBLIC DYNAMICSVIEWER METHODS -----
		void handle_data(double timestamp, int postsynapticNeuronID, float _potential, float _current, float _threshold) {
			if (postsynapticNeuronID == neuron_tracker) {
                while (atomic_guard.test_and_set(std::memory_order_acquire)) {}
				if (!is_closed) {
                    // saving data points to plot
                    if (current_plot) {
                        current_points.append(QPointF(timestamp,_current));
                    }
                    
					points.append(QPointF(timestamp, _potential));
					thres_points.append(QPointF(timestamp, _threshold));
                    // membrane potential axis
					min_y = std::min(min_y, _potential);
					max_y = std::max(max_y, _potential);
                    // injected current axis
                    min_y_right = std::min(min_y_right, _current);
                    max_y_right = std::max(max_y_right, _current);
				} else {
					points.clear();
                    thres_points.clear();
                    current_points.clear();
				}
				atomic_guard.clear(std::memory_order_release);
			}
            
            // time axis
            max_x = timestamp;
		}
		
		// ----- SETTERS -----
		void set_time_window(double new_window) {
            time_window = new_window;
        }
		
		void hardware_acceleration(bool accelerate) {
            openGL = accelerate;
        }
		
        void track_neuron(int neuronToTrack) {
            neuron_tracker = neuronToTrack;
        }
		
        void plot_currents(bool _current_plot) {
            current_plot = _current_plot;
        }
        
        void reset() {
            points.clear();
            thres_points.clear();
            current_points.clear();
        }
        
    Q_SIGNALS:
    public slots:
		
        // ----- QT-RELATED METHODS -----
        void change_tracked_neuron(int new_neuron) {
            if (neuron_tracker != new_neuron) {
                neuron_tracker = new_neuron;
                min_y = -70;
                max_y = -50;
                min_y_right = 0;
                max_y_right = 1;
            }
        }
    
        void disable() {
            while (atomic_guard.test_and_set(std::memory_order_acquire)) {}
            is_closed = true;
            atomic_guard.clear(std::memory_order_release);
        }
    
        void update(QtCharts::QValueAxis *axisX, QtCharts::QValueAxis *axisY, QtCharts::QAbstractSeries *series,  int seriesType) {
            if (!is_closed) {
                if (series) {
                    while (atomic_guard.test_and_set(std::memory_order_acquire)) {}
                    if (openGL) {
                        series->setUseOpenGL(true);
                    }
					
                    switch (seriesType) {
                        case 0:
                            axisX->setRange(max_x - time_window, max_x+1);
                            if (!points.isEmpty()) {
                                auto firstToKeep = std::upper_bound(points.begin(), points.end(), points.back().x() - time_window, [](double timestamp, const QPointF& point) {
                                    return timestamp < point.x();
                                });
                                points.remove(0, static_cast<int>(std::distance(points.begin(), firstToKeep)));
                    
                                static_cast<QtCharts::QXYSeries *>(series)->replace(points);
                                axisY->setRange(min_y-1,max_y+1);
                            }
                            break;
                        case 1:
                            if (!thres_points.isEmpty()) {
                                auto firstToKeep = std::upper_bound(thres_points.begin(), thres_points.end(), thres_points.back().x() - time_window, [](double timestamp, const QPointF& thres_points) {
                                    return timestamp < thres_points.x();
                                });
                                thres_points.remove(0, static_cast<int>(std::distance(thres_points.begin(), firstToKeep)));
                    
                                static_cast<QtCharts::QXYSeries *>(series)->replace(thres_points);
                            }
                            break;
                        case 2:
                            if (current_plot) {
                                if (!current_points.isEmpty()) {
                                    auto firstToKeep = std::upper_bound(current_points.begin(), current_points.end(), current_points.back().x() - time_window, [](double timestamp, const QPointF& current_points) {
                                        return timestamp < current_points.x();
                                    });
                                    current_points.remove(0, static_cast<int>(std::distance(current_points.begin(), firstToKeep)));
                                    
                                    static_cast<QtCharts::QXYSeries *>(series)->replace(current_points);
                                    axisY->setRange(min_y_right-1,max_y_right+1);
                                }
                                break;
                            }
                    }
					
                    atomic_guard.clear(std::memory_order_release);
                }
            }
        }
        
    protected:
		
        // ----- IMPLEMENTATION VARIABLES -----
        bool                  is_closed;
        bool                  openGL;
        double                time_window;
        QVector<QPointF>      points;
        QVector<QPointF>      thres_points;
        QVector<QPointF>      current_points;
        double                max_x;
        float                 min_y;
        float                 max_y;
        float                 min_y_right;
        float                 max_y_right;
        std::atomic_flag      atomic_guard;
        int                   neuron_tracker;
        bool                  current_plot;
    };
}
