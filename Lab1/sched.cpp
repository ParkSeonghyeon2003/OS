/*
*    DKU Operating System Lab
*    Lab1 (Scheduler Algorithm Simulator)
*    Student id : 32221741 
*    Student name : Park Seonghyeon 
*/

#include <cmath>
#include <string>
#include <stdio.h>
#include <iostream>
#include <queue>
#include <vector>
#include <algorithm>
#include "sched.h"

class SPN : public Scheduler{
    private:
    // 하나의 작업이 완료될 때, job_queue_에서 arrival time이 current time 이하인 job들을
    // 뽑아와 벡터 v에 저장
    std::vector<Job> v;
    
    // v에서 service time이 긴 순서대로 정렬
    // service time이 같다면 name 큰 순서로 정렬
    // pop_back()시 shortest job이 반환된다.
    static bool compare(const Job& j1, const Job& j2) {
        if (j1.service_time == j2.service_time) {
            return j1.name > j2.name;
        } else {
            return j1.service_time > j2.service_time;
        }
    }
    public:
        SPN(std::queue<Job> jobs, double switch_overhead) : Scheduler(jobs, switch_overhead) {
            name = "SPN";
        }

        // 스케줄링 함수
        int run() override {
            // 할당된 작업이 없고, job_queue가 비어있지 않으면 작업 할당
            if (current_job_.name == 0 && !job_queue_.empty()) {
                current_job_ = job_queue_.front();
                job_queue_.pop();
            }

            // 현재 작업이 모두 완료되면
            if (current_job_.remain_time == 0) {
                // 작업 완료 시간 기록
                current_job_.completion_time = current_time_;
                // 작업 완료 벡터에 저장
                end_jobs_.push_back(current_job_);

                // 남은 작업이 없으면 종료
                if (v.empty() && job_queue_.empty()) return -1;

                // 새로운 작업 할당
                // 현재까지 도착한 작업을 저장하여 짧은 service time이 뒤로 가도록 정렬
                while (!job_queue_.empty()) {
                    Job front = job_queue_.front();
                    if (front.arrival_time <= current_time_) {
                        v.push_back(front);
                        job_queue_.pop();
                    } else break;
                }
                sort(v.begin(), v.end(), compare);
                current_job_ = v.back();
                v.pop_back();
            
                // context switch 타임 추가
                current_time_ += switch_time_;
            }

            // 현재 작업이 처음 스케줄링 되는 것이라면
            if (current_job_.service_time == current_job_.remain_time) {
                // 첫 실행 시간 기록
                current_job_.first_run_time = current_time_;
            }

            // 현재 시간 ++
            current_time_++;
            // 작업의 남은 시간 --
            current_job_.remain_time--;
            // 스케줄링할 작업명 반환
            return current_job_.name;
        }
};

class RR : public Scheduler{
    private:
        int time_slice_;
        int left_slice_;
        std::queue<Job> waiting_queue;
        /*
        * 구현 (멤버 변수/함수 추가 및 삭제 가능)
        */
    public:
        RR(std::queue<Job> jobs, double switch_overhead, int time_slice) : Scheduler(jobs, switch_overhead) {
            name = "RR_"+std::to_string(time_slice);
            /*
            * 위 생성자 선언 및 이름 초기화 코드 수정하지 말것.
            * 나머지는 자유롭게 수정 및 작성 가능 (아래 코드 수정 및 삭제 가능)
            */
            time_slice_ = time_slice; 
            left_slice_ = time_slice;
        }

        int run() override {
            // 할당된 작업이 없고, job_queue가 비어있지 않으면 작업 할당
            if (current_job_.name == 0 && !job_queue_.empty()) {
                current_job_ = job_queue_.front();
                job_queue_.pop();
            }
            while (!job_queue_.empty()) {
                Job front = job_queue_.front();
                if (front.arrival_time <= current_time_) {
                    waiting_queue.push(front);
                    job_queue_.pop();
                } else break;
            }
            if (left_slice_ == 0 || current_job_.remain_time == 0) {
                
                if (current_job_.remain_time == 0) {
                    // 작업 완료 시간 기록
                    current_job_.completion_time = current_time_;
                    // 작업 완료 벡터에 저장
                    end_jobs_.push_back(current_job_);
                    if (waiting_queue.empty() && job_queue_.empty()) return -1;
                } else if (!waiting_queue.empty()) {
                    // 대기 큐가 비어있지 않으면 현재 작업 대기 큐에 push
                    waiting_queue.push(current_job_);
                }
                // 아래 조건 false면 문맥 교환이 필요 X
                if (!waiting_queue.empty()) {
                    current_job_ = waiting_queue.front();
                    waiting_queue.pop();
                    current_time_ += switch_time_;
                }
                
                // left slice을 time slice 값으로 초기화
                left_slice_ = time_slice_;
            }
            // 현재 작업이 처음 스케줄링 되는 것이라면
            if (current_job_.service_time == current_job_.remain_time) {
                // 첫 실행 시간 기록
                current_job_.first_run_time = current_time_;
            }

            // 현재 시간 ++
            current_time_++;
            // 작업의 남은 시간 --
            current_job_.remain_time--;
            
            left_slice_--;
            // 스케줄링할 작업명 반환
            return current_job_.name;
        }
};

class SRT : public Scheduler{
    private:
        std::vector<Job> v;
        static bool compare (const Job& j1, const Job& j2) {
            if (j1.remain_time == j2.remain_time) {
                return j1.name > j2.name;
            } else {
                return j1.remain_time > j2.remain_time;
            }
        }
    public:
        SRT(std::queue<Job> jobs, double switch_overhead) : Scheduler(jobs, switch_overhead) {
            name = "SRT";
            /*
            * 위 생성자 선언 및 이름 초기화 코드 수정하지 말것.
            * 나머지는 자유롭게 수정 및 작성 가능
            */
        }

        int run() override {
            // 할당된 작업이 없고, job_queue가 비어있지 않으면 작업 할당
            if (current_job_.name == 0 && !job_queue_.empty()) {
                current_job_ = job_queue_.front();
                job_queue_.pop();
            }
            while (!job_queue_.empty()) {
                Job front = job_queue_.front();
                if (front.arrival_time <= current_time_) {
                    v.push_back(front);
                    job_queue_.pop();
                } else break;
            }
            // 현재 작업이 모두 완료되면
            if (current_job_.remain_time == 0) {
                // 작업 완료 시간 기록
                current_job_.completion_time = current_time_;
                // 작업 완료 벡터에 저장
                end_jobs_.push_back(current_job_);
                // 남은 작업이 없으면 종료
                if (job_queue_.empty() && v.empty()) return -1;
                sort(v.begin(), v.end(), compare);
                // 대기열에 작업 존재 시 그 작업 수행
                if (!v.empty()) {
                    current_job_ = v.back();
                    v.pop_back();
                    current_time_ += switch_time_;
                }
            // 도착한 작업 중 remain time이 더 짧은게 있다면
            } else if (!v.empty() && v.front().remain_time < current_job_.remain_time) {
                Job temp = current_job_;
                current_job_ = v.back();
                v.pop_back();
                v.push_back(temp);
                current_time_ += switch_time_;
            }
            // 현재 작업이 처음 스케줄링 되는 것이라면
            if (current_job_.service_time == current_job_.remain_time) {
                // 첫 실행 시간 기록
                current_job_.first_run_time = current_time_;
            }
            current_time_++;
            current_job_.remain_time--;
            return current_job_.name;
        }
};

class HRRN : public Scheduler{
    private:
        struct Waiting_job {
            Job job;
            double waiting_time;
        };
        std::vector<Waiting_job> v;
        static bool compare (const Waiting_job& w1, const Waiting_job& w2) {
            double rr1 = (w1.waiting_time+w1.job.service_time)/w1.job.service_time;
            double rr2 = (w2.waiting_time+w2.job.service_time)/w2.job.service_time;
            if (rr1 == rr2) {
                return w1.job.name > w2.job.name;
            } else {
                return rr1 < rr2;
            }
        }
    public:
        HRRN(std::queue<Job> jobs, double switch_overhead) : Scheduler(jobs, switch_overhead) {
            name = "HRRN";
            /*
            * 위 생성자 선언 및 이름 초기화 코드 수정하지 말것.
            * 나머지는 자유롭게 수정 및 작성 가능
            */            
        }
        int run() override {
            // 할당된 작업이 없고, job_queue가 비어있지 않으면 작업 할당
            if (current_job_.name == 0 && !job_queue_.empty()) {
                current_job_ = job_queue_.front();
                job_queue_.pop();
            }
            while (!job_queue_.empty()) {
                Job front = job_queue_.front();
                if (front.arrival_time <= current_time_) {
                    Waiting_job temp;
                    temp.job = front;
                    temp.waiting_time = current_time_ - temp.job.arrival_time;
                    v.push_back(temp);
                    job_queue_.pop();
                } else break;
            }
            // waiting time 업데이트
            for (int i = 0; i < v.size(); i++) {
                v[i].waiting_time = current_time_ - v[i].job.arrival_time;
            }
            if (current_job_.remain_time == 0) {
                // 작업 완료 시간 기록
                current_job_.completion_time = current_time_;
                // 작업 완료 벡터에 저장
                end_jobs_.push_back(current_job_);
                if (v.empty() && job_queue_.empty()) return -1;
                sort(v.begin(), v.end(), compare);
                current_job_ = v.back().job;
                v.pop_back();
                current_time_ += switch_time_;
            }
            // 현재 작업이 처음 스케줄링 되는 것이라면
            if (current_job_.service_time == current_job_.remain_time) {
                // 첫 실행 시간 기록
                current_job_.first_run_time = current_time_;
            }
            current_time_++;
            current_job_.remain_time--;
            return current_job_.name;
        }
};

// FeedBack 스케줄러 (queue 개수 : 4 / boosting 없음)
class FeedBack : public Scheduler{
    private:
        struct job_with_lt {
            Job job;
            int left_slice;
        };
        std::queue<job_with_lt> queues[4];
        bool is_2i_;
        int time_slice_;
        int left_slice_;
        int curLevel;
        bool isEmpty() {
            for (int i = 0; i < 4; i++) {
                if (!queues[i].empty()) {
                    return false;
                }
            }
            return true;
        }
    public:
        FeedBack(std::queue<Job> jobs, double switch_overhead, bool is_2i) : Scheduler(jobs, switch_overhead) {
            if(is_2i){
                name = "FeedBack_2i";
            } else {
                name = "FeedBack_1";
            }
            is_2i_ = is_2i;
            /*
            * 위 생성자 선언 및 이름 초기화 코드 수정하지 말것.
            * 나머지는 자유롭게 수정 및 작성 가능
            */
        }

        int run() override {
            // 할당된 작업이 없고, job_queue가 비어있지 않으면 작업 할당
            if (current_job_.name == 0 && !job_queue_.empty()) {
                current_job_ = job_queue_.front();
                time_slice_ = 1;
                left_slice_ = time_slice_;
                curLevel = 0;
                job_queue_.pop();
            }
            // 상위 큐부터 채우기
            while(!job_queue_.empty()) {
                Job front = job_queue_.front();
                if (front.arrival_time <= current_time_) {
                    job_with_lt temp;
                    temp.job = front;
                    temp.left_slice = 1;
                    queues[0].push(temp);
                    job_queue_.pop();
                } else break;
            }
            // 현재 작업 완료 혹은 time_slice_가 0이면 문맥 교환
            if (current_job_.remain_time == 0 || left_slice_ <= 0) {
                job_with_lt cur;
                cur.job = current_job_;
                cur.left_slice = is_2i_ ? time_slice_*2 : time_slice_;
                if (current_job_.remain_time == 0) {
                    // 작업 완료 시간 기록
                    current_job_.completion_time = current_time_;
                    // 작업 완료 벡터에 저장
                    end_jobs_.push_back(current_job_);
                    
                    if (isEmpty() && job_queue_.empty()) return -1;
                } else if (!isEmpty()) {
                    if (curLevel < 3) queues[curLevel+1].push(cur);
                    else queues[curLevel].push(cur);
                }
                if (!isEmpty()) {
                    for (int i = 0; i < 4; i++) {
                        if (!queues[i].empty()) {
                            job_with_lt front = queues[i].front();
                            current_job_ = front.job;
                            time_slice_ = front.left_slice;
                            left_slice_ = time_slice_;
                            curLevel = i;
                            queues[i].pop();
                            if (current_job_.name != cur.job.name) current_time_ += switch_time_;
                            break;
                        }
                    }
                }
            }
            
            // 현재 작업이 처음 스케줄링 되는 것이라면
            if (current_job_.service_time == current_job_.remain_time) {
                // 첫 실행 시간 기록
                current_job_.first_run_time = current_time_;
            }
            current_time_++;
            current_job_.remain_time--;
            left_slice_--;
            return current_job_.name;
        }
};
